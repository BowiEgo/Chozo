#include <Runtime/RHI/Texture.h>

namespace CZ {

class TextureObj {

public:
    TextureObj(const TextureSpecification& spec) : m_Spec(spec) {}

    TextureObj(const TextureSpecification& spec, Image image) : m_Spec(spec), m_Image(image) {}

    virtual ~TextureObj() {
        CZ_CORE_LOG(Error, "TextureObj destructed.");
        m_Image.Destroy();
    }

    std::string GetName() const { return m_Spec.Name; }

    TextureType GetType() const { return m_Spec.Type; }

    Extent2D GetSize() const { return m_Spec.Size; }

    PixelFormat GetFormat() const { return m_Spec.Format; }

    TextureUsage GetUsage() const { return m_Spec.Usage; }

    Image GetImage() { return m_Image; }

protected:
    TextureSpecification m_Spec;
    Image m_Image;
};

} // namespace CZ