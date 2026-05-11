#include <Runtime/RHI/Texture.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogTexture, Info);

void Texture::Destroy(Texture texture) {
    if (!texture) return;

    Delete(texture.Unwrap());
}

} // namespace CZ