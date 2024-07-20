#pragma once

#include "Chozo/Core/Input.h"

namespace Chozo {
    class MacInput : public Input
    {
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) override;

        virtual bool IsMouseButtonPressedImpl(MouseButton button) override;
        virtual std::pair<float, float> GetMousePositionImpl() override;
        virtual float GetMouseXImpl() override;
        virtual float GetMouseYImpl() override;
    };
}