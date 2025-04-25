#pragma once

namespace Chozo {

    class Observer
    {
    public:
        ~Observer() = default;
        void OnUpdated();
    };
}