#pragma once

#include "CoreExport.h"
#include "CoreMinimal.h"

#include <atomic>
#include <future>
#include <queue>
#include <thread>

class CORE_API CThreadPool {
public:
    explicit CThreadPool(size_t threadCount = std::thread::hardware_concurrency()) : m_Stop(false) {
        Start(threadCount);
    }

    ~CThreadPool() { Stop(); }

    template <typename F, typename... Args>
    auto Submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
        using ReturnType = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            if (m_Stop) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            m_Tasks.emplace([task]() { (*task)(); });
        }

        m_Condition.notify_one();
        return result;
    }

    template <typename F, typename... Args>
    auto SubmitBatch(size_t count, F&& f, Args&&... args)
        -> std::vector<std::future<typename std::invoke_result_t<F, Args...>>> {

        std::vector<std::future<typename std::invoke_result_t<F, Args...>>> results;
        results.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            results.push_back(Submit(f, args...));
        }

        return results;
    }

private:
    void Start(size_t threadCount);

public:
    void Restart(size_t threadCount);
    void Stop();

    void WaitAll();
    size_t PendingTaskCount();

    size_t GetThreadCount() const { return m_Workers.size(); }
    bool IsIdle() const { return m_Tasks.empty() && m_ActiveTasks == 0; }

private:
    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_Tasks;

    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    std::atomic<bool> m_Stop{ false };
    std::atomic<size_t> m_ActiveTasks{ 0 };
};