#include <Core/JobSystem/JobSystem.h>
#include <Core/Log/LogMacros.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Profiler/TracyProfiler.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace CZ {

/// thread safe ring buffer to store JobHeader info
class JobQueue {
public:
    JobQueue(size_t capacity) : m_Cap(capacity), m_Head(0), m_Tail(0) {
        m_Jobs = (JobHeader*)HeapMalloc(sizeof(JobHeader) * capacity, MEMORY_USAGE_JOB_SYSTEM);
    }

    ~JobQueue() { HeapFree(m_Jobs); }

    inline bool Enqueue(const JobHeader& job) {
        bool success = false;

        {
            std::unique_lock<std::mutex> lock(m_Mutex);

            size_t next = (m_Head + 1) % m_Cap;

            if (next != m_Tail) {
                m_Jobs[m_Head] = job;
                m_Head         = next;
                m_Size.fetch_add(1);
                success = true;
            }
        }

        return success;
    }

    inline bool dequeue(JobHeader& job) {
        bool success = false;

        {
            std::unique_lock<std::mutex> lock(m_Mutex);

            if (m_Head != m_Tail) {
                job    = m_Jobs[m_Tail];
                m_Tail = (m_Tail + 1) % m_Cap;
                m_Size.fetch_sub(1);
                success = true;
            }
        }

        return success;
    }

    inline size_t size() { return m_Size; }

    inline bool empty() { return m_Size == 0; }

    void prioritize(uint32_t prioType) {
        std::unique_lock<std::mutex> lock(m_Mutex);

        size_t front = m_Tail;

        for (size_t now = m_Tail; now != m_Head; now = (now + 1) % m_Cap) {
            if (m_Jobs[now].Type == prioType) {
                JobHeader tmp = m_Jobs[front];
                m_Jobs[front] = m_Jobs[now];
                m_Jobs[now]   = tmp;

                front = (front + 1) % m_Cap;
            }
        }
    }

private:
    std::mutex m_Mutex;
    std::atomic<size_t> m_Size;
    const size_t m_Cap;
    size_t m_Head;
    size_t m_Tail;
    JobHeader* m_Jobs;
};

struct WorkerThread {
    std::thread Handle;
    std::atomic_bool IsWorking;
};

/// @brief thread-based job system implementation
struct JobSystemObj {
    std::condition_variable WakeCV;    // CV to wake worker threads
    std::condition_variable WaitAllCV; // CV to wait until all jobs complete
    std::mutex WakeMutex;              // used in conjunction with WakeCV
    std::mutex WaitAllMutex;           // used in conjunction with WaitAllCV
    std::vector<WorkerThread*> WorkerThreads;
    std::atomic<bool> IsRunning;
    std::atomic<size_t> JobCounter;
    JobQueue ImmQueue;
    JobQueue StdQueue;

    JobSystemObj(const JobSystemInfo& info);
    ~JobSystemObj();
};

static JobSystemObj* s_Obj;

static void WorkerThreadMain(void* thread);

static void ExecuteJob(const JobHeader& job) {
    CZ_PROFILE_SCOPE;

    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    job.OnExecute(job.User);

    if (job.OnComplete) job.OnComplete(job.User);

    if (s_Obj->JobCounter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        std::unique_lock<std::mutex> lock(s_Obj->WaitAllMutex);
        s_Obj->WaitAllCV.notify_one();
    }
}

JobSystemObj::JobSystemObj(const JobSystemInfo& info)
    : ImmQueue(info.ImmediateQueueCapacity), StdQueue(info.StandardQueueCapacity) {
    int workerCount = (int)std::thread::hardware_concurrency() - 1;
    WorkerThreads.resize(workerCount);

    for (int i = 0; i < workerCount; i++) {
        WorkerThreads[i]            = CZ_NEW(MEMORY_USAGE_JOB_SYSTEM, WorkerThread);
        WorkerThreads[i]->IsWorking = false;
    }

    IsRunning = true;
}

JobSystemObj::~JobSystemObj() {
    IsRunning = false;

    WakeCV.notify_all();

    for (WorkerThread* thread : WorkerThreads) {
        thread->Handle.join();
        Delete<WorkerThread>(thread);
    }
}

static void WorkerThreadMain(void* user) {
    WorkerThread* thread = (WorkerThread*)user;

    while (s_Obj->IsRunning) {
        JobHeader job;
        bool foundJob = s_Obj->ImmQueue.dequeue(job) || s_Obj->StdQueue.dequeue(job);

        if (foundJob) {
            thread->IsWorking = true;
            ExecuteJob(job);
            thread->IsWorking = false;
        } else {
            // put worker thread to sleep
            std::unique_lock<std::mutex> lock(s_Obj->WakeMutex);
            s_Obj->WakeCV.wait(lock, [] {
                bool hasJob = !s_Obj->ImmQueue.empty() || !s_Obj->StdQueue.empty();
                return hasJob || !s_Obj->IsRunning;
            });
        }
    }
}

void JobSystem::Init(const JobSystemInfo& info) {
    if (s_Obj) return;

    s_Obj = CZ_NEW(MEMORY_USAGE_JOB_SYSTEM, JobSystemObj, info);

    for (WorkerThread* thread : s_Obj->WorkerThreads)
        thread->Handle = std::thread(&WorkerThreadMain, thread);
}

void JobSystem::Shutdown() {
    if (!s_Obj) return;

    Delete<JobSystemObj>(s_Obj);
    s_Obj = nullptr;
}

JobSystem JobSystem::Get() { return { s_Obj }; }

int JobSystem::GetWorkerThreadCount() { return (int)m_Obj->WorkerThreads.size(); }

void JobSystem::WaitAll() {
    CZ_PROFILE_SCOPE;

    bool allDispatched;

    do {
        m_Obj->WakeCV.notify_one();

        allDispatched = m_Obj->ImmQueue.empty() && m_Obj->StdQueue.empty();
    } while (!allDispatched);

    std::unique_lock<std::mutex> lock(m_Obj->WaitAllMutex);
    m_Obj->WaitAllCV.wait(lock,
                          [] { return s_Obj->JobCounter.load(std::memory_order_acquire) == 0; });
}

void JobSystem::Submit(const JobHeader* job, JobDispatchType type) {
    JobQueue* queue = (type == JOB_DISPATCH_IMMEDIATE) ? &m_Obj->ImmQueue : &m_Obj->StdQueue;

    if (queue->Enqueue(*job)) {
        s_Obj->JobCounter.fetch_add(1);
        s_Obj->WakeCV.notify_one();
    } else {
        s_Obj->JobCounter.fetch_add(1);

        // TODO: currently it is possible that the main thread
        //       starts executing a large job and freezes the app.
        ExecuteJob(*job);
    }
}

void JobSystem::Prioritize(uint32_t type) {
    m_Obj->ImmQueue.prioritize(type);
    m_Obj->StdQueue.prioritize(type);
}

} // namespace CZ