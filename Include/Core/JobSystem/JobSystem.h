#pragma once

#include <Core/Header/Handle.hpp>

#include <cstdint>
#include <cstdlib>

namespace CZ {

enum JobDispatchType {
    JOB_DISPATCH_IMMEDIATE = 0,
    JOB_DISPATCH_STANDARD,
};

typedef void (*JobFn)(void* user);

struct JobHeader {
    uint32_t Type;
    JobFn OnExecute;  // job body, runs on worker thread
    JobFn OnComplete; // optional hook, runs on worker thread after job body
    void* User;       // dependency injection during callbacks
};

struct JobSystemInfo {
    size_t ImmediateQueueCapacity;
    size_t StandardQueueCapacity;
};

/// @brief Thread-based Job System, using one main thread and multiple worker threads.
///        All threads may create new jobs, but only the main thread can wait for jobs to finish.
struct JobSystem : Handle<struct JobSystemObj> {
    static void Init(const JobSystemInfo& info);
    static void Shutdown();

    /// @brief get singleton handle
    /// @return job system handle
    static JobSystem Get();

    /// @brief get number of worker threads
    int GetWorkerThreadCount();

    /// @brief wait for worker threads to complete all jobs
    /// @warning can only be called from main thread,
    ///          worker threads waiting for jobs may
    ///          lead to deadlocks.
    void WaitAll();

    void Submit(const JobHeader* job, JobDispatchType type);

    /// @brief Move jobs of the specified type to the very front of each job queue.
    ///        Note that the immediate queue still takes priority over other queues.
    /// @param type the job type to prioritize
    void Prioritize(uint32_t type);
};

} // namespace CZ
