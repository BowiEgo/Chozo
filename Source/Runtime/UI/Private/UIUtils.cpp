#include "UIUtils.h"

#include "RHIAPI.h"
#include "VFS.h"

#include "nanosvg.h"
#include "nanosvgrast.h"

#include "imgui_internal.h"

#include "stb_image.h"
#include "stb_image_resize2.h"

DEFINE_LOG_CATEGORY(LogUIUtils);

static NSVGrasterizer* g_Rasterizer = nullptr;

static std::string MiddleTruncateByChar(const std::string& filename, float maxWidth) {
    std::string originName = filename;

    float avgCharWidth = ImGui::CalcTextSize("A").x;
    if (avgCharWidth <= 0) avgCharWidth = 8.0f;

    size_t dotPos    = filename.find_last_of('.');
    std::string ext  = (dotPos != std::string::npos) ? filename.substr(dotPos) : "";
    std::string name = (dotPos != std::string::npos) ? filename.substr(0, dotPos) : filename;

    float extWidth     = ImGui::CalcTextSize(ext.c_str()).x;
    float nameMaxWidth = maxWidth - extWidth;
    if (nameMaxWidth <= 0) return "..." + ext;

    int maxNameChars = (int)(nameMaxWidth / avgCharWidth);
    int keepChars    = maxNameChars - 3;
    if (keepChars <= 0) return "..." + ext;

    int nameLen = (int)name.length();
    keepChars   = std::min(keepChars, nameLen);

    int leftChars  = keepChars / 2;
    int rightChars = keepChars - leftChars;
    rightChars     = std::min(rightChars, nameLen - leftChars);

    std::string result;
    if (leftChars > 0) result.append(name, 0, leftChars);
    result.append("...");
    if (rightChars > 0) result.append(name, name.length() - rightChars, rightChars);
    result.append(ext);
    return result;
}

namespace ChozoUtils::UI {

void InitSVGLoader() {
    if (!g_Rasterizer) {
        g_Rasterizer = nsvgCreateRasterizer();
    }
}

void ShutdownSVGLoader() {
    if (g_Rasterizer) {
        nsvgDeleteRasterizer(g_Rasterizer);
        g_Rasterizer = nullptr;
    }
}

TRef<CTexture> LoadSVGIcon(const std::string& name, int targetSize, uint32 strokeColor) {
    if (!g_Rasterizer) {
        InitSVGLoader();
    }

    std::filesystem::path svgPath = VFS::Resolve("svgs://" + name + ".svg");
    std::string pathString        = svgPath.string();

    NSVGimage* svg = nsvgParseFromFile(pathString.c_str(), "px", 96.0f);
    if (!svg) {
        CZ_LOG(LogUIUtils, Error, "Failed to load SVG: {}", pathString);
        return nullptr;
    }

    for (NSVGshape* shape = svg->shapes; shape; shape = shape->next) {
        shape->stroke.type  = NSVG_PAINT_COLOR;
        shape->stroke.color = strokeColor;

        if (shape->strokeWidth == 0) {
            shape->strokeWidth = 1.0f;
        }
    }

    float scale = (float)targetSize / (float)svg->width;
    int w       = (int)(svg->width * scale);
    int h       = (int)(svg->height * scale);

    unsigned char* data = (unsigned char*)malloc(w * h * 4);
    if (!data) {
        CZ_LOG(LogUIUtils, Error, "Failed to allocate image buffer");
        nsvgDelete(svg);
        return nullptr;
    }

    nsvgRasterize(g_Rasterizer, svg, 0, 0, scale, data, w, h, w * 4);

    FTextureSpecification spec;
    spec.Name   = name;
    spec.Size   = { (uint32)w, (uint32)h };
    spec.Format = EPixelFormat::RGBA8_UNORM;
    spec.Usage  = ETextureUsage::Texture;

    FBuffer imageData(data, w * h * 4);
    TRef<CTexture> texture = CreateRef<CTexture>(spec, imageData);

    free(data);
    nsvgDelete(svg);

    if (!texture) {
        CZ_LOG(LogUIUtils, Error, "Failed to create texture from SVG");
    }

    CZ_LOG(LogUIUtils, Info, "Loaded SVG icon: {} ({}x{})", name, w, h);
    return texture;
}

unsigned char* LoadImagePreview(const char* path, int max_size, int* out_w, int* out_h) {
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
    if (!data) return nullptr;

    int target_w = w;
    int target_h = h;

    if (w > max_size || h > max_size) {
        float scale = (float)max_size / std::max(w, h);
        target_w    = (int)(w * scale);
        target_h    = (int)(h * scale);
    }

    target_w = std::min(target_w, 16384);
    target_h = std::min(target_h, 16384);

    if (target_w == w && target_h == h) {
        *out_w = w;
        *out_h = h;
        return data;
    }

    unsigned char* resized = (unsigned char*)malloc(target_w * target_h * 4);

    stbir_resize_uint8_srgb(data, w, h, 0, resized, target_w, target_h, 0, STBIR_RGBA);

    stbi_image_free(data);

    *out_w = target_w;
    *out_h = target_h;
    return resized;
}

std::vector<std::string> GetWrappedFileName(const char* label, float WrapWidth, float MaxLineHeight,
                                            float RowSpacing, ImVec2 RawTextSize) {
    // CZ_LOG(LogUIUtils, Info, "GetWrappedFileName: {}", label);

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiContext& g   = *GImGui;

    std::vector<std::string> lines;
    if (RawTextSize.y <= MaxLineHeight * 1.5f) {
        // [Note] Single line
        lines.push_back(label);
    } else {
        // [Note] Multi-line logic: split into two lines
        // Line 1: Fill as much as possible
        const char* line1_end =
            g.Font->CalcWordWrapPositionA(1.0f, label, label + strlen(label), WrapWidth);
        lines.push_back(std::string(label, line1_end));

        // Line 2: The rest, with middle truncation
        std::string remaining = line1_end;
        // Trim leading spaces for the second line
        remaining.erase(0, remaining.find_first_not_of(' '));
        lines.push_back(MiddleTruncateByChar(remaining, WrapWidth));
    }

    return lines;
}

} // namespace ChozoUtils::UI