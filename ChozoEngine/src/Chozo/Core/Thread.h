#pragma once

#include <string>
#include <thread>

namespace Chozo {

    class Thread
    {
    public:
        explicit Thread(std::string  name);
        ~Thread();

        template<typename Fn, typename ...Args>
        void Dispatch(Fn&& func, Args&&... args)
        {
            auto threadFunc = [this, func = std::forward<Fn>(func)](Args&&... args) {
                SetName(m_Name);
                func(std::forward<Args>(args)...);
            };
            m_Thread = std::thread(threadFunc, std::forward<Args>(args)...);
        }

        void SetName(const std::string& name);
        std::string GetName();

        void Join();
    private:
        std::string m_Name;
        std::thread m_Thread;
    };

    class ThreadSignal
    {
    public:
        explicit ThreadSignal(const std::string& name, bool manualReset = false);

        void Wait();
        void Signal();
        void Reset();
    private:
        void* m_SignalHandle = nullptr;
    };
}