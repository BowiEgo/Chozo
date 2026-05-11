#include <Runtime/RHI/Image.h>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogImage, Info);

void Image::Destroy(Image image) {
    if (!image) return;

    Delete(image.Unwrap());
}

} // namespace CZ