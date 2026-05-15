#pragma once

#include <Runtime/RHI/Image.h>

namespace CZ {

class ImageObj {
public:
    ImageObj(const ImageSpecification& spec) : m_Spec(spec) {}
    virtual ~ImageObj() = default;

protected:
    ImageSpecification m_Spec;

    bool m_IsValid = true;
};

} // namespace CZ
