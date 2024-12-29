#pragma once

#include "czpch.h"

namespace Chozo {

    template<typename T>
    class ObservableProperty
    {
    public:
        using Callback = std::function<void(const T&)>;

        ObservableProperty(const T& value = T()) : value(value) {}

        void Set(const T& newValue)
        {
            value = newValue;
            if (onChange) onChange(value);
        }

        const T& Get() const { return value; }

        void SetOnChange(Callback callback) { onChange = callback; }

    private:
        T value;
        Callback onChange;
    };
}

