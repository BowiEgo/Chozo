#include <Core/JobSystem/JobSystem.h>

#include <doctest/doctest.h>

#include <atomic>
#include <thread>
#include <vector>

using namespace CZ;

namespace {

struct JobCounters {
    std::atomic<int> Executed{ 0 };
    std::atomic<int> Completed{ 0 };
    std::atomic<int> Sum{ 0 };
};

void CountExecution(void* user) { static_cast<JobCounters*>(user)->Executed.fetch_add(1); }

void CountCompletion(void* user) { static_cast<JobCounters*>(user)->Completed.fetch_add(1); }

void Accumulate(void* user) {
    auto* counters = static_cast<JobCounters*>(user);
    counters->Sum.fetch_add(1);
}

JobSystemInfo TestInfo() {
    JobSystemInfo info;
    info.ImmediateQueueCapacity = 8;
    info.StandardQueueCapacity  = 16;
    return info;
}

} // namespace

TEST_SUITE("JobSystem") {

    TEST_CASE("Runs every submitted job and waits for completion") {
        if (std::thread::hardware_concurrency() == 0) {
            // The implementation derives the worker count from this value (see P1-7).
            MESSAGE("skipped: hardware_concurrency() is not available");
            return;
        }

        JobSystem::Init(TestInfo());
        REQUIRE(!!JobSystem::Get());

        JobCounters counters;
        std::vector<JobHeader> jobs(64);

        for (auto& job : jobs) {
            job            = JobHeader{};
            job.Type       = 0;
            job.OnExecute  = CountExecution;
            job.OnComplete = CountCompletion;
            job.User       = &counters;
        }

        for (auto& job : jobs) {
            JobSystem::Get().Submit(&job, JOB_DISPATCH_STANDARD);
        }

        JobSystem::Get().WaitAll();

        CHECK_EQ(counters.Executed.load(), 64);
        CHECK_EQ(counters.Completed.load(), 64);

        JobSystem::Get().Shutdown();
        CHECK_FALSE(!!JobSystem::Get());
    }

    TEST_CASE("More jobs than the queue can hold still all run") {
        if (std::thread::hardware_concurrency() == 0) {
            MESSAGE("skipped: hardware_concurrency() is not available");
            return;
        }

        JobSystem::Init(TestInfo());
        REQUIRE(!!JobSystem::Get());

        JobCounters counters;
        std::vector<JobHeader> jobs(128); // queue capacity is 16 -> exercises the inline fallback

        for (auto& job : jobs) {
            job           = JobHeader{};
            job.Type      = 0;
            job.OnExecute = Accumulate;
            job.User      = &counters;
        }

        for (auto& job : jobs) {
            JobSystem::Get().Submit(&job, JOB_DISPATCH_STANDARD);
        }

        JobSystem::Get().WaitAll();

        CHECK_EQ(counters.Sum.load(), 128);

        JobSystem::Get().Shutdown();
    }

    TEST_CASE("Immediate and standard queues are both drained") {
        if (std::thread::hardware_concurrency() == 0) {
            MESSAGE("skipped: hardware_concurrency() is not available");
            return;
        }

        JobSystem::Init(TestInfo());
        REQUIRE(!!JobSystem::Get());

        JobCounters counters;
        JobHeader immediate{};
        immediate.Type      = 1;
        immediate.OnExecute = CountExecution;
        immediate.User      = &counters;

        JobHeader standard{};
        standard.Type      = 2;
        standard.OnExecute = CountExecution;
        standard.User      = &counters;

        JobSystem::Get().Submit(&immediate, JOB_DISPATCH_IMMEDIATE);
        JobSystem::Get().Submit(&standard, JOB_DISPATCH_STANDARD);
        JobSystem::Get().WaitAll();

        CHECK_EQ(counters.Executed.load(), 2);

        JobSystem::Get().Shutdown();
    }

    TEST_CASE("Worker thread count is reported") {
        if (std::thread::hardware_concurrency() == 0) {
            MESSAGE("skipped: hardware_concurrency() is not available");
            return;
        }

        JobSystem::Init(TestInfo());
        REQUIRE(!!JobSystem::Get());

        CHECK_GE(JobSystem::Get().GetWorkerThreadCount(), 0);

        JobSystem::Get().Shutdown();
    }

    TEST_CASE("Jobs submitted from several threads are all accounted for") {
        if (std::thread::hardware_concurrency() == 0) {
            MESSAGE("skipped: hardware_concurrency() is not available");
            return;
        }

        // Regression net for the submit/counter ordering: registering the job after publishing it
        // let a worker complete it first, which wrapped the outstanding-job counter and made
        // WaitAll() wait forever.
        JobSystem::Init(TestInfo());
        REQUIRE(!!JobSystem::Get());

        JobCounters counters;
        constexpr int kSubmitters    = 4;
        constexpr int kJobsPerThread = 64;

        std::vector<std::vector<JobHeader>> jobStorage(kSubmitters);
        std::vector<std::thread> submitters;

        for (int t = 0; t < kSubmitters; ++t) {
            jobStorage[t].resize(kJobsPerThread);
            for (auto& job : jobStorage[t]) {
                job           = JobHeader{};
                job.Type      = 0;
                job.OnExecute = Accumulate;
                job.User      = &counters;
            }

            submitters.emplace_back([&, t] {
                for (auto& job : jobStorage[t]) {
                    JobSystem::Get().Submit(&job, JOB_DISPATCH_STANDARD);
                }
            });
        }

        for (auto& submitter : submitters)
            submitter.join();

        JobSystem::Get().WaitAll();

        CHECK_EQ(counters.Sum.load(), kSubmitters * kJobsPerThread);

        JobSystem::Get().Shutdown();
    }

    TEST_CASE("Init and Shutdown are idempotent") {
        if (std::thread::hardware_concurrency() == 0) {
            MESSAGE("skipped: hardware_concurrency() is not available");
            return;
        }

        JobSystem::Init(TestInfo());
        JobSystem::Init(TestInfo()); // second Init is ignored
        REQUIRE(!!JobSystem::Get());

        JobSystem::Get().Shutdown();
        JobSystem::Shutdown(); // second Shutdown is ignored
        CHECK_FALSE(!!JobSystem::Get());
    }
}
