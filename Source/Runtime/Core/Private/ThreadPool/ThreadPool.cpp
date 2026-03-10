#include "ThreadPool.h"

void CThreadPool::WaitAll() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Condition.wait(lock, [this]() { return m_Tasks.empty() && m_ActiveTasks == 0; });
}

size_t CThreadPool::PendingTaskCount() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    return m_Tasks.size();
}

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

void CThreadPool::Stop() {
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Stop = true;
    }

    m_Condition.notify_all();

    for (auto& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
