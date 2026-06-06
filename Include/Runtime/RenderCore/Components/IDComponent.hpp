#pragma once

#include <Core/Header/UUID.hpp>

namespace CZ {

struct IDComponent {
    UUID ID;

    IDComponent()                   = default;
    IDComponent(const IDComponent&) = default;
};

} // namespace CZ
