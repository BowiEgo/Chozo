#include "ThreadPool.h"

void CThreadPool::Start(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        m_Workers.emplace_back([this, i] {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    m_Condition.wait(lock, [this] { return m_Stop || !m_Tasks.empty(); });

                    if (m_Stop && m_Tasks.empty()) {
                        return;
                    }

                    task = std::move(m_Tasks.front());
                    m_Tasks.pop();
                    ++m_ActiveTasks;
                }

                task();

                {
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    --m_ActiveTasks;
                    if (m_Tasks.empty() && m_ActiveTasks == 0) {
                        m_Condition.notify_all();
                    }
                }
            }
        });
    }
}
void CThreadPool::Restart(size_t threadCount) {
    std::unique_lock<std::mutex> lock(m_Mutex);

    // [Note] Prevent starting if already running
    if (!m_Stop) return;

    m_Stop = false;
    m_Workers.reserve(threadCount);

    Start(threadCount);
}

void CThreadPool::Stop() {
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_Stop) return; // Already stopped
        m_Stop = true;
    }

    // [Note] Wake up all threads to let them exit
    m_Condition.notify_all();

    for (auto& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    // [Note] Clear resources so Start() can be called again
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Workers.clear();
    while (!m_Tasks.empty())
        m_Tasks.pop(); // Optional: Clear remaining tasks
    m_ActiveTasks = 0;
}

void CThreadPool::WaitAll() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Condition.wait(lock, [this]() { return m_Tasks.empty() && m_ActiveTasks == 0; });
}

size_t CThreadPool::PendingTaskCount() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    return m_Tasks.size();
}
