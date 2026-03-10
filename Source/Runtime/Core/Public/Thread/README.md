# CThreadPool Documentation

## 📋 Overview

`CThreadPool` is a modern C++ thread pool implementation that provides a simple and easy-to-use interface for managing concurrent task execution. It automatically manages the lifecycle of worker threads and supports core features such as task submission, batch processing, and completion waiting.

## ✨ Features

- **Automatic Thread Management**: Threads are automatically started in the constructor and cleaned up in the destructor
- **Flexible Task Submission**: Supports any callable object (functions, lambdas, function objects)
- **Type-Safe Future Returns**: Obtain task results via `std::future`
- **Batch Task Processing**: Submit multiple identical tasks with one call
- **Thread Safety**: Internal mutexes and condition variables ensure thread safety
- **Graceful Shutdown**: Waits for all tasks to complete before exiting in destructor

## 🔧 Basic Usage

### 1. Include Header

```cpp
#include "ThreadPool.h"
```

### 2. Create Thread Pool

```cpp
// Use default thread count (CPU core count)
CThreadPool pool;

// Specify thread count (e.g., 4 threads)
CThreadPool pool(4);
```

### 3. Submit Tasks

```cpp
// Submit task without parameters
auto future1 = pool.Submit([] {
    return "Hello from thread pool!";
});

// Submit task with parameters
auto future2 = pool.Submit([](int a, int b) {
    return a + b;
}, 10, 20);

// Submit void task
pool.Submit([] {
    std::cout << "Background task" << std::endl;
});
```

### 4. Get Results

```cpp
// Wait for task to complete and get result
std::string result1 = future1.get();  // "Hello from thread pool!"
int result2 = future2.get();           // 30
```

## 📦 Batch Task Processing

```cpp
// Submit multiple identical tasks in batch
std::vector<int> data = {1, 2, 3, 4, 5};
auto futures = pool.SubmitBatch(data.size(), [](int x) {
    return x * x;
}, 0);  // The 0 here will be used for each task

// Wait for all tasks and collect results
std::vector<int> squares;
for (auto& future : futures) {
    squares.push_back(future.get());
}
// squares = {1, 4, 9, 16, 25}
```

Note: In batch submission, the same arguments are passed to each task. If you need different arguments per task, use a loop with individual Submit calls.

## ⏱️ Task Management

### Wait for All Tasks to Complete

```cpp
// Block until all submitted tasks are finished
pool.WaitAll();
```

### Check Pending Tasks

```cpp
// Get number of tasks waiting in queue
size_t pending = pool.PendingTaskCount();
```

### Check Thread Count

```cpp
// Get number of worker threads
size_t threadCount = pool.GetThreadCount();
```

### Check if Idle

```cpp
// Check if there are no tasks running or pending
bool idle = pool.IsIdle();
```

## 🎯 Examples

### File Thumbnail Loading

```cpp
class UFileDialog {
private:
    CThreadPool m_ThumbPool{4};  // 4 threads for thumbnail loading
    std::mutex m_ResultMutex;
    std::queue<std::pair<int, FRawFileImage>> m_PendingThumbs;

public:
    void RequestThumbnails() {
        for (size_t i = 0; i < m_Files.size(); ++i) {
            m_ThumbPool.Submit([this, i] {
                auto thumb = GetFileThumbnail(m_Files[i].Path, 128);
                if (thumb.Data) {
                    std::lock_guard<std::mutex> lock(m_ResultMutex);
                    m_PendingThumbs.push({i, std::move(thumb)});
                }
            });
        }
    }

    void ProcessThumbnails() {
        std::lock_guard<std::mutex> lock(m_ResultMutex);
        while (!m_PendingThumbs.empty()) {
            auto [idx, thumb] = std::move(m_PendingThumbs.front());
            m_PendingThumbs.pop();
            UpdateTexture(idx, thumb);
        }
    }
};
```

### Parallel Data Processing

```cpp
std::vector<int> ProcessDataInParallel(const std::vector<int>& input) {
    CThreadPool pool;
    std::vector<std::future<int>> futures;

    // Submit each item for processing
    for (int value : input) {
        futures.push_back(pool.Submit([](int x) {
            // Simulate heavy computation
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return x * x;
        }, value));
    }

    // Collect results
    std::vector<int> results;
    for (auto& future : futures) {
        results.push_back(future.get());
    }

    return results;
}
```

### Task with Timeout

```cpp
auto future = pool.Submit([] {
    // Long running task
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 42;
});

// Wait with timeout
if (future.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    std::cout << "Task timeout!" << std::endl;
} else {
    std::cout << "Result: " << future.get() << std::endl;
}
```

## ⚠️ Important Notes

1. Exception Safety: Exceptions thrown inside tasks are captured and re-thrown when calling get() on the future
2. Thread Safety: The thread pool itself is thread-safe, but ensure your task data is properly synchronized
3. Resource Management: Always ensure the thread pool outlives any futures that depend on it
4. Stop State: Once Stop() is called (or destructor runs), no new tasks can be submitted

## 🔍 Thread Count Recommendations

| Scenario          | Recommended Thread Count            |
| :---------------- | :---------------------------------- |
| CPU-bound tasks   | std::thread::hardware_concurrency() |
| I/O-bound tasks   | 2-4× CPU cores                      |
| Thumbnail loading | 4 threads                           |
| Default (mixed)   | std::thread::hardware_concurrency() |

## 📊 Performance Tips

1. Task Granularity: Tasks should be coarse enough to overcome threading overhead (typically >10,000 operations)
2. Batch Processing: Use SubmitBatch for many similar small tasks
3. Avoid Blocking: Don't call get() on futures within submitted tasks (can cause deadlocks)
4. Thread Count: For file I/O operations, using slightly more threads than CPU cores can improve throughput

## 🚨 Common Pitfalls

### Don't Do This

```cpp
// ❌ Deadlock: Waiting for task inside another task
pool.Submit([&pool] {
    auto fut = pool.Submit([] { return 42; });
    int result = fut.get();  // This will deadlock!
});

// ❌ Using pool after destruction
auto* pool = new CThreadPool();
auto fut = pool->Submit([] { return 42; });
delete pool;
int result = fut.get();  // Undefined behavior!
```

### Do This Instead

```cpp
// ✅ Proper task chaining
pool.Submit([] {
    // Do work without waiting for other pool tasks
});

// ✅ Ensure pool outlives futures
CThreadPool pool;
auto fut = pool.Submit([] { return 42; });
// pool will be destroyed after fut is used
int result = fut.get();
```
