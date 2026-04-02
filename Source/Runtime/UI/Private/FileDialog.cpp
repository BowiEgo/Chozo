#include "FileDialog.h"

#include "IconManager.h"
#include "RHIAPI.h"
#include "SystemUtils.h"
#include "UIUtils.h"

#ifndef CZ_PLATFORM_WINDOWS
    #include <pwd.h>
    #include <unistd.h>
#endif

#include "imgui_internal.h"
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

DEFINE_LOG_CATEGORY(LogFileDialog);

#define ICON_SIZE (ImGui::GetFontSize() + 3)
#define GUI_ELEMENT_SIZE ((std::max)(ImGui::GetFontSize() + 10.f, 24.f))
#define PI 3.141592f

static std::unordered_map<std::string, std::vector<std::string>> s_WrappedFileNameMap;

/* UI CONTROLS */
bool FolderNode(const char* label, ImTextureID icon, bool& clicked, bool default_open = false) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    clicked = false;

    ImU32 id = window->GetID(label);
    int opened = window->StateStorage.GetInt(id, default_open ? 1 : 0);
    ImVec2 pos = window->DC.CursorPos;
    const bool is_mouse_x_over_arrow =
        (g.IO.MousePos.x >= pos.x && g.IO.MousePos.x < pos.x + g.FontSize);
    if (ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, g.FontSize + g.Style.FramePadding.y * 2))) {
        if (is_mouse_x_over_arrow) {
            int* p_opened = window->StateStorage.GetIntRef(id, 0);
            opened = *p_opened = !*p_opened;
        } else {
            clicked = true;
        }
    }
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
    if (doubleClick && hovered) {
        int* p_opened = window->StateStorage.GetIntRef(id, 0);
        opened = *p_opened = !*p_opened;
        clicked = false;
    }
    if (hovered || active)
        window->DrawList->AddRectFilled(
            g.LastItemData.Rect.Min, g.LastItemData.Rect.Max,
            ImGui::ColorConvertFloat4ToU32(
                ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

    // Icon, text
    float icon_posX = pos.x + g.FontSize + g.Style.FramePadding.y;
    float text_posX = icon_posX + g.Style.FramePadding.y + ICON_SIZE;
    ImGui::RenderArrow(window->DrawList, ImVec2(pos.x, pos.y + g.Style.FramePadding.y),
                       ImGui::ColorConvertFloat4ToU32(
                           ImGui::GetStyle().Colors[((hovered && is_mouse_x_over_arrow) || opened)
                                                        ? ImGuiCol_Text
                                                        : ImGuiCol_TextDisabled]),
                       opened ? ImGuiDir_Down : ImGuiDir_Right);
    window->DrawList->AddImage(icon, ImVec2(icon_posX, pos.y),
                               ImVec2(icon_posX + ICON_SIZE, pos.y + ICON_SIZE));
    ImGui::RenderText(ImVec2(text_posX, pos.y + g.Style.FramePadding.y), label);
    if (opened) ImGui::TreePush(label);
    return opened != 0;
}

bool FileNode(const char* label, ImTextureID icon) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // ImU32 id = window->GetID(label);
    ImVec2 pos = window->DC.CursorPos;
    bool ret =
        ImGui::InvisibleButton(label, ImVec2(-FLT_MIN, g.FontSize + g.Style.FramePadding.y * 2));

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered || active)
        window->DrawList->AddRectFilled(
            g.LastItemData.Rect.Min, g.LastItemData.Rect.Max,
            ImGui::ColorConvertFloat4ToU32(
                ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]));

    // Icon, text
    window->DrawList->AddImage(icon, ImVec2(pos.x, pos.y),
                               ImVec2(pos.x + ICON_SIZE, pos.y + ICON_SIZE));
    ImGui::RenderText(
        ImVec2(pos.x + g.Style.FramePadding.y + ICON_SIZE, pos.y + g.Style.FramePadding.y), label);

    return ret;
}

bool PathBox(const char* label, std::filesystem::path& path, char* pathBuffer, ImVec2 size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    bool ret = false;
    const ImGuiID id = window->GetID(label);
    int* state = window->StateStorage.GetIntRef(id, 0);

    ImGui::SameLine();

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 uiPos = ImGui::GetCursorPos();
    ImVec2 size = ImGui::CalcItemSize(size_arg, 200.0f, GUI_ELEMENT_SIZE);
    const ImRect bb(pos, pos + size);

    // buttons
    if (!(*state & 0b001)) {
        ImGui::PushClipRect(bb.Min, bb.Max, false);

        // background
        bool hovered = g.IO.MousePos.x >= bb.Min.x && g.IO.MousePos.x <= bb.Max.x &&
                       g.IO.MousePos.y >= bb.Min.y && g.IO.MousePos.y <= bb.Max.y;
        bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        bool anyOtherHC = false; // are any other items hovered or clicked?
        window->DrawList->AddRectFilled(
            pos, pos + size,
            ImGui::ColorConvertFloat4ToU32(
                ImGui::GetStyle()
                    .Colors[(*state & 0b10) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]));

        // fetch the buttons (so that we can throw some away if needed)
        std::vector<std::string> btnList;
        float totalWidth = 0.0f;
        for (auto comp : path) {
            std::string section = comp.string();
            if (section.size() == 1 && (section[0] == '\\' || section[0] == '/')) continue;

            totalWidth += ImGui::CalcTextSize(section.c_str()).x + style.FramePadding.x * 2.0f +
                          GUI_ELEMENT_SIZE;
            btnList.push_back(section);
        }
        totalWidth -= GUI_ELEMENT_SIZE;

        // UI buttons
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        bool isFirstElement = true;
        for (size_t i = 0; i < btnList.size(); i++) {
            if (totalWidth > size.x - 30 &&
                i != btnList.size() - 1) { // trim some buttons if there's not enough space
                float elSize = ImGui::CalcTextSize(btnList[i].c_str()).x +
                               style.FramePadding.x * 2.0f + GUI_ELEMENT_SIZE;
                totalWidth -= elSize;
                continue;
            }

            ImGui::PushID(static_cast<int>(i));
            if (!isFirstElement) {
                ImGui::ArrowButtonEx("##dir_dropdown", ImGuiDir_Right,
                                     ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));
                anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
                ImGui::SameLine();
            }
            if (ImGui::Button(btnList[i].c_str(), ImVec2(0, GUI_ELEMENT_SIZE))) {
#ifdef CZ_PLATFORM_WINDOWS
                std::string newPath = "";
#else
                std::string newPath = "/";
#endif
                for (size_t j = 0; j <= i; j++) {
                    newPath += btnList[j];
#ifdef CZ_PLATFORM_WINDOWS
                    if (j != i) newPath += "\\";
#else
                    if (j != i) newPath += "/";
#endif
                }
                path = std::filesystem::path(newPath);
                ret = true;
            }
            anyOtherHC |= ImGui::IsItemHovered() | ImGui::IsItemClicked();
            ImGui::SameLine();
            ImGui::PopID();

            isFirstElement = false;
        }
        ImGui::PopStyleVar(2);

        // click state
        if (!anyOtherHC && clicked) {
            strcpy(pathBuffer, path.string().c_str());
            *state |= 0b001;
            *state &= 0b011; // remove SetKeyboardFocus flag
        } else
            *state &= 0b110;

        // hover state
        if (!anyOtherHC && hovered && !clicked)
            *state |= 0b010;
        else
            *state &= 0b101;

        ImGui::PopClipRect();

        // allocate space
        ImGui::SetCursorPos(uiPos);
        ImGui::ItemSize(size);
    }
    // input box
    else {
        bool skipActiveCheck = false;
        if (!(*state & 0b100)) {
            skipActiveCheck = true;
            ImGui::SetKeyboardFocusHere();
            if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) *state |= 0b100;
        }
        if (ImGui::InputTextEx("##pathbox_input", "", pathBuffer, 1024, size_arg,
                               ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string tempStr(pathBuffer);
            if (std::filesystem::exists(tempStr)) path = std::filesystem::path(tempStr);
            ret = true;
        }
        if (!skipActiveCheck && !ImGui::IsItemActive()) *state &= 0b010;
    }

    return ret;
}

bool FavoriteButton(const char* label, bool isFavorite) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    ImVec2 pos = window->DC.CursorPos;
    bool ret = ImGui::InvisibleButton(label, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE));

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    float size = g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x;

    int numPoints = 5;
    float innerRadius = (size - 10.0f) / 4;
    float outerRadius = (size - 10.0f) / 2;
    float angle = PI / numPoints;
    ImVec2 center = ImVec2(pos.x + size / 2, pos.y + size / 2);

    // fill
    if (isFavorite || hovered || active) {
        ImU32 fillColor = 0xff00ffff; //
        ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
        if (hovered || active)
            fillColor = ImGui::ColorConvertFloat4ToU32(
                ImGui::GetStyle().Colors[active ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered]);

        // since there is no PathFillConcave, fill first the inner part, then the triangles
        // inner
        window->DrawList->PathClear();
        for (int i = 1; i < numPoints * 2; i += 2)
            window->DrawList->PathLineTo(ImVec2(center.x + innerRadius * sin(i * angle),
                                                center.y - innerRadius * cos(i * angle)));
        window->DrawList->PathFillConvex(fillColor);

        // triangles
        for (int i = 0; i < numPoints; i++) {
            window->DrawList->PathClear();

            int pIndex = i * 2;
            window->DrawList->PathLineTo(ImVec2(center.x + outerRadius * sin(pIndex * angle),
                                                center.y - outerRadius * cos(pIndex * angle)));
            window->DrawList->PathLineTo(
                ImVec2(center.x + innerRadius * sin((pIndex + 1) * angle),
                       center.y - innerRadius * cos((pIndex + 1) * angle)));
            window->DrawList->PathLineTo(
                ImVec2(center.x + innerRadius * sin((pIndex - 1) * angle),
                       center.y - innerRadius * cos((pIndex - 1) * angle)));

            window->DrawList->PathFillConvex(fillColor);
        }
    }

    // outline
    window->DrawList->PathClear();
    for (int i = 0; i < numPoints * 2; i++) {
        float radius = i & 1 ? innerRadius : outerRadius;
        window->DrawList->PathLineTo(
            ImVec2(center.x + radius * sin(i * angle), center.y - radius * cos(i * angle)));
    }
    window->DrawList->PathStroke(
        ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), true, 2.0f);

    return ret;
}

bool FileIcon(const char* label, bool isSelected, ImTextureID icon, ImVec2 size, bool hasPreview,
              int previewWidth, int previewHeight) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float windowSpace = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 pos = window->DC.CursorPos;
    bool ret = false;

    if (ImGui::InvisibleButton(label, size)) ret = true;

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    bool doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
    if (doubleClick && hovered) ret = true;

    float iconSize = size.y - g.FontSize * 2;
    float iconPosX = pos.x + (size.x - iconSize) / 2.0f;

    if (hovered || active || isSelected)
        window->DrawList->AddRectFilled(
            g.LastItemData.Rect.Min, g.LastItemData.Rect.Max,
            ImGui::ColorConvertFloat4ToU32(
                ImGui::GetStyle()
                    .Colors[active ? ImGuiCol_HeaderActive
                                   : (isSelected ? ImGuiCol_Header : ImGuiCol_HeaderHovered)]));

    if (hasPreview) {
        ImVec2 availSize = ImVec2(size.x, iconSize);

        float scale = std::min<float>(availSize.x / previewWidth, availSize.y / previewHeight);
        availSize.x = previewWidth * scale;
        availSize.y = previewHeight * scale;

        float previewPosX = pos.x + (size.x - availSize.x) / 2.0f;
        float previewPosY = pos.y + (iconSize - availSize.y) / 2.0f;

        window->DrawList->AddImage(icon, ImVec2(previewPosX, previewPosY),
                                   ImVec2(previewPosX + availSize.x, previewPosY + availSize.y));
    } else
        window->DrawList->AddImage(icon, ImVec2(iconPosX, pos.y),
                                   ImVec2(iconPosX + iconSize, pos.y + iconSize));

    float wrapWidth = size.x - g.Style.ItemSpacing.x;
    float maxLineHeight = g.FontSize;
    float rowSpacing = 2.0f;
    ImVec2 rawTextSize = ImGui::CalcTextSize(label, nullptr, true, wrapWidth);

    std::vector<std::string> lines;

    auto itrCache = s_WrappedFileNameMap.find(label);
    if (itrCache != s_WrappedFileNameMap.end()) {
        lines = itrCache->second;
    } else {
        lines = ChozoUtils::UI::GetWrappedFileName(label, wrapWidth, maxLineHeight, rowSpacing,
                                                   rawTextSize);
        s_WrappedFileNameMap[label] = lines;
    }

    float totalTextHeight =
        std::min((float)lines.size(), 2.0f) * maxLineHeight + (lines.size() > 1 ? rowSpacing : 0);
    float textOffY = ((size.y - iconSize) - totalTextHeight) * 0.5f;
    float currentY = pos.y + iconSize + textOffY;

    window->DrawList->PushClipRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, true);
    for (size_t i = 0; i < std::min(lines.size(), (size_t)2); ++i) {
        ImVec2 lineSize = ImGui::CalcTextSize(lines[i].c_str());
        // [Note] Calculate X for horizontal centering of THIS specific line
        float lineX = pos.x + (size.x - lineSize.x) * 0.5f;

        window->DrawList->AddText(g.Font, g.FontSize, ImVec2(lineX, currentY),
                                  ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]),
                                  lines[i].c_str());
        currentY += maxLineHeight + rowSpacing;
    }
    window->DrawList->PopClipRect();

    float lastButtomPos = ImGui::GetItemRectMax().x;
    float thisButtonPos = lastButtomPos + style.ItemSpacing.x +
                          size.x; // Expected position if next button was on same line
    if (thisButtonPos < windowSpace) ImGui::SameLine();

    return ret;
}

FileData::FileData(const std::filesystem::path& path) {
    std::error_code ec;
    Path = path;
    IsDirectory = std::filesystem::is_directory(path, ec);
    Size = std::filesystem::file_size(path, ec);

    std::string pathStr = path.string();

    struct stat attr;
    stat(pathStr.c_str(), &attr);
    DateModified = attr.st_ctime;

    Thumbnail = nullptr;
    ThumbnailHeight = 0;
    ThumbnailWidth = 0;
}

UFileDialog::UFileDialog(IRHIContext* context) : m_GraphicContext(context) {
    m_IsOpen = false;
    m_Type = 0;
    m_CalledOpenPopup = false;
    m_SortColumn = 0;
    m_SortDirection = ImGuiSortDirection_Ascending;
    m_FilterSelection = 0;
    m_InputTextbox[0] = 0;
    m_PathBuffer[0] = 0;
    m_SearchBuffer[0] = 0;
    m_NewEntryBuffer[0] = 0;
    m_SelectedFileItem = -1;
    m_Zoom = 1.0f;

    m_ThumbnailLoader = nullptr;
    m_ThumbnailLoaderRunning = false;

    SetDirectory(std::filesystem::current_path(), false);

    // favorites are available on every OS
    FileTreeNode* quickAccess = new FileTreeNode("Quick Access");
    quickAccess->Read = true;
    m_TreeCache.push_back(quickAccess);

    std::error_code ec;

    // Quick Access
    std::string homePath;

#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
    if (home) {
        homePath = home;
    } else {
        const char* drive = std::getenv("HOMEDRIVE");
        const char* path = std::getenv("HOMEPATH");
        if (drive && path) {
            homePath = std::string(drive) + std::string(path);
        } else {
            homePath = "C:\\";
        }
    }
#else
    const char* home = std::getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(geteuid());
        if (pw) home = pw->pw_dir;
    }
    if (home) {
        homePath = home;
    } else {
        homePath = "/";
    }
#endif

    homePath = std::filesystem::path(homePath).make_preferred().string();

#ifdef CZ_PLATFORM_WINDOWS
    const std::vector<std::string> commonDirs = { "Desktop", "Documents", "Downloads", "Pictures",
                                                  "Music",   "Videos",    "OneDrive" };
#elif CZ_PLATFORM_LINUX
    const std::vector<std::string> commonDirs = {
        "Desktop", "Documents", "Downloads", "Pictures", "Music", "Videos", "Public", "Templates"
    };
#elif CZ_PLATFORM_MACOS

    const std::vector<std::string> commonDirs = { "Desktop",  "Documents",   "Downloads",
                                                  "Pictures", "Music",       "Movies",
                                                  "Public",   "Applications" };
#endif

    if (std::filesystem::exists(homePath, ec)) {
        quickAccess->Children.push_back(new FileTreeNode(homePath));
    }

    for (const auto& dir : commonDirs) {
        std::string fullPath = homePath + "/" + dir;
        if (std::filesystem::exists(fullPath, ec)) {
            quickAccess->Children.push_back(new FileTreeNode(fullPath));
        }
    }

    // This Computer
    FileTreeNode* thisPC = new FileTreeNode("This Computer");
    thisPC->Read = true;

#ifdef CZ_PLATFORM_WINDOWS
    DWORD d = GetLogicalDrives();
    for (int i = 0; i < 26; i++)
        if (d & (1 << i))
            thisPC->Children.push_back(new FileTreeNode(std::string(1, 'A' + i) + ":\\"));
#elif CZ_PLATFORM_MACOS
    for (const auto& entry : std::filesystem::directory_iterator("/", ec))
        if (std::filesystem::is_directory(entry, ec))
            thisPC->Children.push_back(new FileTreeNode(entry.path().string()));
#endif
    m_TreeCache.push_back(thisPC);
}

UFileDialog::~UFileDialog() { Shutdown(); }

bool UFileDialog::Save(const std::string& key, const std::string& title, const std::string& filter,
                       const std::string& startingDir) {
    if (!m_CurrentKey.empty()) return false;

    m_CurrentKey = key;
    m_CurrentTitle = title + "###" + key;
    m_IsOpen = true;
    m_CalledOpenPopup = false;
    m_Result.clear();
    m_InputTextbox[0] = 0;
    m_Selections.clear();
    m_SelectedFileItem = -1;
    m_IsMultiselect = false;
    m_Type = IFD_DIALOG_SAVE;

    ParseFilter(filter);
    if (!startingDir.empty())
        SetDirectory(std::filesystem::path(startingDir), false);
    else
        SetDirectory(m_CurrentDirectory, false); // refresh contents

    return true;
}

bool UFileDialog::Open(const std::string& key, const std::string& title, const std::string& filter,
                       bool isMultiselect, const std::string& startingDir) {
    if (!m_CurrentKey.empty()) return false;

    m_CurrentKey = key;
    m_CurrentTitle = title + "###" + key;
    m_IsOpen = true;
    m_CalledOpenPopup = false;
    m_Result.clear();
    m_InputTextbox[0] = 0;
    m_Selections.clear();
    m_SelectedFileItem = -1;
    m_IsMultiselect = isMultiselect;
    m_Type = filter.empty() ? IFD_DIALOG_DIRECTORY : IFD_DIALOG_FILE;

    ParseFilter(filter);
    if (!startingDir.empty())
        SetDirectory(std::filesystem::path(startingDir), false);
    else
        SetDirectory(m_CurrentDirectory, false); // refresh contents

    return true;
}

bool UFileDialog::IsDone(const std::string& key) {
    bool isMe = m_CurrentKey == key;

    if (!m_TextureGarbage.empty()) {
        const int MAX_CLEANUP_PER_FRAME = 20;
        int cleanedCount = 0;

        auto it = m_TextureGarbage.begin();
        while (it != m_TextureGarbage.end() && cleanedCount < MAX_CLEANUP_PER_FRAME) {
            it = m_TextureGarbage.erase(it);
            cleanedCount++;
        }
    }

    if (isMe && m_IsOpen) {
        if (!m_CalledOpenPopup) {
            ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
            ImGui::OpenPopup(m_CurrentTitle.c_str());
            m_CalledOpenPopup = true;
        }

        if (ImGui::BeginPopupModal(m_CurrentTitle.c_str(), &m_IsOpen,
                                   ImGuiWindowFlags_NoScrollbar)) {
            RenderFileDialog();
            ImGui::EndPopup();
        } else
            m_IsOpen = false;
    }

    return isMe && !m_IsOpen;
}

void UFileDialog::Close() {
    m_CurrentKey.clear();
    m_BackHistory = std::stack<std::filesystem::path>();
    m_ForwardHistory = std::stack<std::filesystem::path>();

    // clear the tree
    for (auto fn : m_TreeCache) {
        for (auto item : fn->Children) {
            for (auto ch : item->Children)
                ClearTree(ch);
            item->Children.clear();
            item->Read = false;
        }
    }

    ClearThumbnails();
    CIconManager::Get(m_GraphicContext).ClearCaches();
}

void UFileDialog::Shutdown() {
    ClearThumbnails();
    m_TextureGarbage.clear();

    for (auto fn : m_TreeCache)
        ClearTree(fn);
    m_TreeCache.clear();
}

void UFileDialog::RemoveFavorite(const std::string& path) {
    auto itr = std::find(m_Favorites.begin(), m_Favorites.end(), m_CurrentDirectory.string());

    if (itr != m_Favorites.end()) m_Favorites.erase(itr);

    // remove from sidebar
    for (auto& p : m_TreeCache)
        if (p->Path == "Quick Access") {
            for (size_t i = 0; i < p->Children.size(); i++)
                if (p->Children[i]->Path == path) {
                    p->Children.erase(p->Children.begin() + i);
                    break;
                }
            break;
        }
}

void UFileDialog::AddFavorite(const std::string& path) {
    if (std::count(m_Favorites.begin(), m_Favorites.end(), path) > 0) return;

    if (!std::filesystem::exists(std::filesystem::path(path))) return;

    m_Favorites.push_back(path);

    // add to sidebar
    for (auto& p : m_TreeCache)
        if (p->Path == "Quick Access") {
            p->Children.push_back(new FileTreeNode(path));
            break;
        }
}

TRef<IRHITexture2D> UFileDialog::CreateTexture(uint8_t* data, int w, int h, char fmt) {
    FTexture2DSpecification spec;
    spec.Name = "UFileDialog_Preview";
    spec.Size = { (uint32_t)w, (uint32_t)h };
    spec.Format = (fmt == 0) ? EPixelFormat::BGRA8_UNORM : EPixelFormat::RGBA8_UNORM;
    spec.Usage = ETextureUsage::Texture;

    FBuffer imageData(data, w * h * 4);
    TRef<IRHITexture2D> texture = IRHIAPI::CreateTexture2D(m_GraphicContext, spec, imageData);

    return texture;
}

void UFileDialog::Select(const std::filesystem::path& path, bool isCtrlDown) {
    bool multiselect = isCtrlDown && m_IsMultiselect;

    if (!multiselect) {
        m_Selections.clear();
        m_Selections.push_back(path);
    } else {
        auto it = std::find(m_Selections.begin(), m_Selections.end(), path);
        if (it != m_Selections.end())
            m_Selections.erase(it);
        else
            m_Selections.push_back(path);
    }

    if (m_Selections.size() == 1) {
        std::string filename = m_Selections[0].filename().string();
        if (filename.size() == 0) filename = m_Selections[0].string(); // drive

        strcpy(m_InputTextbox, filename.c_str());
    } else {
        std::string textboxVal = "";
        for (const auto& sel : m_Selections) {
            std::string filename = sel.filename().string();
            if (filename.size() == 0) filename = sel.string();

            textboxVal += "\"" + filename + "\", ";
        }
        strcpy(m_InputTextbox, textboxVal.substr(0, textboxVal.size() - 2).c_str());
    }
}

bool UFileDialog::Finalize(const std::string& filename) {
    bool hasResult =
        (!filename.empty() && m_Type != IFD_DIALOG_DIRECTORY) || m_Type == IFD_DIALOG_DIRECTORY;

    if (hasResult) {
        if (!m_IsMultiselect || m_Selections.size() <= 1) {
            std::filesystem::path path = std::filesystem::path(filename);
            if (path.is_absolute())
                m_Result.push_back(path);
            else
                m_Result.push_back(m_CurrentDirectory / path);
            if (m_Type == IFD_DIALOG_DIRECTORY || m_Type == IFD_DIALOG_FILE) {
                if (!std::filesystem::exists(m_Result.back())) {
                    m_Result.clear();
                    return false;
                }
            }
        } else {
            for (const auto& sel : m_Selections) {
                if (sel.is_absolute())
                    m_Result.push_back(sel);
                else
                    m_Result.push_back(m_CurrentDirectory / sel);
                if (m_Type == IFD_DIALOG_DIRECTORY || m_Type == IFD_DIALOG_FILE) {
                    if (!std::filesystem::exists(m_Result.back())) {
                        m_Result.clear();
                        return false;
                    }
                }
            }
        }

        if (m_Type == IFD_DIALOG_SAVE) {
            // add the extension
            if (m_FilterSelection < m_FilterExtensions.size() &&
                m_FilterExtensions[m_FilterSelection].size() > 0) {
                if (!m_Result.back().has_extension()) {
                    std::string extAdd = m_FilterExtensions[m_FilterSelection][0];
                    m_Result.back().replace_extension(extAdd);
                }
            }
        }
    }

    m_IsOpen = false;

    return true;
}

void UFileDialog::ParseFilter(const std::string& filter) {
    m_Filter = "";
    m_FilterExtensions.clear();
    m_FilterSelection = 0;

    if (filter.empty()) return;

    std::vector<std::string> exts;

    size_t lastSplit = 0, lastExt = 0;
    bool inExtList = false;
    for (size_t i = 0; i < filter.size(); i++) {
        if (filter[i] == ',') {
            if (!inExtList)
                lastSplit = i + 1;
            else {
                exts.push_back(filter.substr(lastExt, i - lastExt));
                lastExt = i + 1;
            }
        } else if (filter[i] == '{') {
            std::string filterName = filter.substr(lastSplit, i - lastSplit);
            if (filterName == ".*") {
                m_Filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
                m_FilterExtensions.push_back(std::vector<std::string>());
            } else
                m_Filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
            inExtList = true;
            lastExt = i + 1;
        } else if (filter[i] == '}') {
            exts.push_back(filter.substr(lastExt, i - lastExt));
            m_FilterExtensions.push_back(exts);
            exts.clear();

            inExtList = false;
        }
    }
    if (lastSplit != 0) {
        std::string filterName = filter.substr(lastSplit);
        if (filterName == ".*") {
            m_Filter += std::string(std::string("All Files (*.*)\0").c_str(), 16);
            m_FilterExtensions.push_back(std::vector<std::string>());
        } else
            m_Filter += std::string((filterName + "\0").c_str(), filterName.size() + 1);
    }
}

TRef<IRHITexture2D> UFileDialog::GetIcon(const std::filesystem::path& path) {
    std::string pathU8 = path.string();

    if (pathU8 == "Quick Access")
        return CIconManager::Get(m_GraphicContext).GetOrLoadSVGIcon("lightning");
    if (pathU8 == "This Computer")
        return CIconManager::Get(m_GraphicContext).GetOrLoadSVGIcon("computer");

    CIconManager::Get(m_GraphicContext).RestartLoading();
    return CIconManager::Get(m_GraphicContext).GetOrLoadFileIcon(path);
}

TRef<IRHITexture2D> UFileDialog::GetThumbnail(const std::filesystem::path& path) {
    {
        std::lock_guard<std::mutex> lock(m_ThumbMutex);
        auto itr = m_ThumbMap.find(path.string());
        if (itr != m_ThumbMap.end()) {
            return itr->second;
        }
    }

    return GetIcon(path);
}

void UFileDialog::RequestThumbnails() {
    if (!m_ThumbMap.empty()) return;

    m_ThumbPool.Restart(4);

    for (size_t i = 0; i < m_Content.size(); i++) {
        auto& data = m_Content[i];
        if (data.IsDirectory) continue;

        auto path = data.Path;

        m_ThumbPool.Submit([this, path] {
            FRawFileImage thumb = ChozoUtils::File::GetFileThumbnail(path, 128);

            CZ_LOG(LogUFileDialog, Trace, "Get Thumbnail: [PathU8]{} [Size]{} [Indice]{}",
                   thumb.PathU8, thumb.Size, thumb.Index);

            if (!thumb.Data) return;

            {
                std::lock_guard<std::mutex> lock(m_ThumbMutex);
                m_PendingRawThumbs.push_back(std::move(thumb));
            }
        });
    }
}

void UFileDialog::ProcessPendingThumbs() {
    std::lock_guard<std::mutex> lock(m_ThumbMutex);

    for (auto& thumb : m_PendingRawThumbs) {
        auto texture = CreateTexture(thumb.Data, thumb.Width, thumb.Height, thumb.Format);
        free(thumb.Data);
        if (texture) {
            m_ThumbMap[thumb.PathU8] = texture;
        }
    }
    m_PendingRawThumbs.clear();
}

void UFileDialog::RefreshThumbnails() {
    m_ThumbPool.Stop();

    if (m_Zoom >= 5.0f) {
        RequestThumbnails();
    }
}

void UFileDialog::ClearThumbnails() {
    for (const auto& [key, thumb] : m_ThumbMap) {
        m_TextureGarbage.push_back(thumb);
    }
    m_ThumbMap.clear();
}

void UFileDialog::ClearTree(FileTreeNode* node) {
    if (node == nullptr) return;

    for (auto n : node->Children)
        ClearTree(n);

    delete node;
    node = nullptr;
}

void UFileDialog::SetDirectory(const std::filesystem::path& p, bool addHistory) {
    bool isSameDir = m_CurrentDirectory == p;

    if (addHistory && !isSameDir) m_BackHistory.push(m_CurrentDirectory);

    m_CurrentDirectory = p;
#ifdef CZ_PLATFORM_WINDOWS
    // drives don't work well without the backslash symbol
    if (p.string().size() == 2 && p.string()[1] == ':')
        m_CurrentDirectory = std::filesystem::path(p.string() + "\\");
#endif

    ClearThumbnails();
    m_Content.clear(); // p == "" after this line, due to reference
    m_SelectedFileItem = -1;

    if (m_Type == IFD_DIALOG_DIRECTORY || m_Type == IFD_DIALOG_FILE) m_InputTextbox[0] = 0;
    m_Selections.clear();

    if (!isSameDir) {
        m_SearchBuffer[0] = 0;
    }

    if (p.string() == "Quick Access") {
        for (auto& node : m_TreeCache) {
            if (node->Path == p)
                for (auto& c : node->Children)
                    m_Content.push_back(FileData(c->Path));
        }
    } else if (p.string() == "This Computer") {
        for (auto& node : m_TreeCache) {
            if (node->Path == p)
                for (auto& c : node->Children)
                    m_Content.push_back(FileData(c->Path));
        }
    } else {
        std::error_code ec;
        if (std::filesystem::exists(m_CurrentDirectory, ec))
            for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory, ec)) {
                if (ChozoUtils::File::IsHiddenOrSystem(entry.path())) continue;
                FileData info(entry.path());

                // skip files when IFD_DIALOG_DIRECTORY
                if (!info.IsDirectory && m_Type == IFD_DIALOG_DIRECTORY) continue;

                // check if filename matches search query
                if (m_SearchBuffer[0]) {
                    std::string filename = info.Path.string();

                    std::string filenameSearch = filename;
                    std::string query(m_SearchBuffer);
                    std::transform(filenameSearch.begin(), filenameSearch.end(),
                                   filenameSearch.begin(), ::tolower);
                    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

                    if (filenameSearch.find(query, 0) == std::string::npos) continue;
                }

                // check if extension matches
                if (!info.IsDirectory && m_Type != IFD_DIALOG_DIRECTORY) {
                    if (m_FilterSelection < m_FilterExtensions.size()) {
                        const auto& exts = m_FilterExtensions[m_FilterSelection];
                        if (exts.size() > 0) {
                            std::string extension = info.Path.extension().string();

                            // extension not found? skip
                            if (std::count(exts.begin(), exts.end(), extension) == 0) continue;
                        }
                    }
                }

                m_Content.push_back(info);
            }
    }

    SortContent(m_SortColumn, m_SortDirection);
    ClearThumbnails();
    RefreshThumbnails();
    CIconManager::Get(m_GraphicContext).StopLoading();
}

void UFileDialog::SortContent(unsigned int column, unsigned int sortDirection) {
    // 0 -> name, 1 -> date, 2 -> size
    m_SortColumn = column;
    m_SortDirection = sortDirection;

    // split into directories and files
    std::partition(m_Content.begin(), m_Content.end(),
                   [](const FileData& data) { return data.IsDirectory; });

    if (m_Content.size() > 0) {
        // find where the file list starts
        size_t fileIndex = 0;
        for (; fileIndex < m_Content.size(); fileIndex++)
            if (!m_Content[fileIndex].IsDirectory) break;

        // compare function
        auto compareFn = [column, sortDirection](const FileData& left,
                                                 const FileData& right) -> bool {
            // name
            if (column == 0) {
                std::string lName = left.Path.string();
                std::string rName = right.Path.string();

                std::transform(lName.begin(), lName.end(), lName.begin(), ::tolower);
                std::transform(rName.begin(), rName.end(), rName.begin(), ::tolower);

                int comp = lName.compare(rName);

                if (sortDirection == ImGuiSortDirection_Ascending) return comp < 0;
                return comp > 0;
            }
            // date
            else if (column == 1) {
                if (sortDirection == ImGuiSortDirection_Ascending)
                    return left.DateModified < right.DateModified;
                else
                    return left.DateModified > right.DateModified;
            }
            // size
            else if (column == 2) {
                if (sortDirection == ImGuiSortDirection_Ascending)
                    return left.Size < right.Size;
                else
                    return left.Size > right.Size;
            }

            return false;
        };

        // sort the directories
        std::sort(m_Content.begin(), m_Content.begin() + fileIndex, compareFn);

        // sort the files
        std::sort(m_Content.begin() + fileIndex, m_Content.end(), compareFn);
    }
}

void UFileDialog::RenderTree(FileTreeNode* node) {
    // directory
    std::error_code ec;
    ImGui::PushID(node);
    bool isClicked = false;
    std::string displayName = node->Path.stem().string();
    if (displayName.size() == 0) displayName = node->Path.string();

    auto tex = GetIcon(node->Path);
    bool isDefaultOpen = displayName == "Quick Access" || displayName == "This Computer";
    if (FolderNode(displayName.c_str(), (ImTextureID)tex->GetDescriptorSet(), isClicked,
                   isDefaultOpen)) {
        if (!node->Read) {
            // cache children if it's not already cached
            if (std::filesystem::exists(node->Path, ec))
                for (const auto& entry : std::filesystem::directory_iterator(node->Path, ec)) {
                    if (std::filesystem::is_directory(entry, ec))
                        node->Children.push_back(new FileTreeNode(entry.path().string()));
                }
            node->Read = true;
        }

        // display children
        for (auto c : node->Children)
            if (c->Path == c->Path.root_path() || !ChozoUtils::File::IsHiddenOrSystem(c->Path))
                RenderTree(c);

        ImGui::TreePop();
    }
    if (isClicked) SetDirectory(node->Path);
    ImGui::PopID();
}

void UFileDialog::RenderContent() {
    CIconManager::Get(m_GraphicContext).ProcessRawIcons();
    ProcessPendingThumbs();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) m_SelectedFileItem = -1;

    // table view
    if (m_Zoom < 5.0f) {
        if (ImGui::BeginTable("##contentTable", 3,
                              /*ImGuiTableFlags_Resizable |*/ ImGuiTableFlags_Sortable,
                              ImVec2(0, -FLT_MIN))) {
            // header
            ImGui::TableSetupColumn("Name##filename", ImGuiTableColumnFlags_WidthStretch,
                                    0.0f - 1.0f, 0);
            ImGui::TableSetupColumn(
                "Date modified##filedate",
                ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 0.0f, 1);
            ImGui::TableSetupColumn(
                "Size##filesize", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize,
                0.0f, 2);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            // sort
            if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                if (sortSpecs->SpecsDirty) {
                    sortSpecs->SpecsDirty = false;
                    SortContent(sortSpecs->Specs->ColumnUserID, sortSpecs->Specs->SortDirection);
                }
            }

            // content
            int fileId = 0;
            for (auto& entry : m_Content) {
                std::string filename = entry.Path.filename().string();
                if (filename.size() == 0) filename = entry.Path.string(); // drive

                bool isSelected = std::count(m_Selections.begin(), m_Selections.end(), entry.Path);
                auto tex = GetIcon(entry.Path);
                float iconTotalSize = ICON_SIZE + 16 * (m_Zoom - 1.0f);
                float lineHeight = ImGui::GetTextLineHeight();
                float verticalTextOffset = (iconTotalSize - lineHeight) * 0.5f;

                ImGui::TableNextRow();
                float rowStartY = ImGui::GetCursorPosY();

                // icon
                ImGui::TableSetColumnIndex(0);
                ImGui::Image((ImTextureID)tex->GetDescriptorSet(),
                             ImVec2(iconTotalSize, iconTotalSize));
                ImGui::SameLine();

                if (ImGui::Selectable(("##" + filename).c_str(), isSelected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowDoubleClick,
                                      ImVec2(0, iconTotalSize))) {
                    std::error_code ec;
                    bool isDir = std::filesystem::is_directory(entry.Path, ec);

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        if (isDir) {
                            SetDirectory(entry.Path);
                            break;
                        } else
                            Finalize(filename);
                    } else {
                        if ((isDir && m_Type == IFD_DIALOG_DIRECTORY) || !isDir)
                            Select(entry.Path, ImGui::GetIO().KeyCtrl);
                    }
                }
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) m_SelectedFileItem = fileId;
                fileId++;

                ImGui::SameLine();
                ImGui::SetCursorPosY(rowStartY + verticalTextOffset);
                ImGui::Text("%s", filename.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::SetCursorPosY(rowStartY + verticalTextOffset);
                auto tm = std::localtime(&entry.DateModified);
                if (tm) {
                    ImGui::Text("%d/%d/%d %02d:%02d", tm->tm_mon + 1, tm->tm_mday,
                                1900 + tm->tm_year, tm->tm_hour, tm->tm_min);
                } else {
                    ImGui::Text("---");
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::SetCursorPosY(rowStartY + verticalTextOffset);
                ImGui::Text("%.3f KiB", entry.Size / 1024.0f);
            }

            ImGui::EndTable();
        }
    }
    // "icon" view
    else {
        // content
        int fileId = 0;
        for (auto& entry : m_Content) {
            std::string filename = entry.Path.filename().string();
            if (filename.size() == 0) filename = entry.Path.string(); // drive

            bool isSelected = std::count(m_Selections.begin(), m_Selections.end(), entry.Path);
            auto tex = entry.IsDirectory ? GetIcon(entry.Path) : GetThumbnail(entry.Path);
            auto texSize = tex->GetSize();

            if (FileIcon(filename.c_str(), isSelected, (ImTextureID)tex->GetDescriptorSet(),
                         ImVec2(32 + 16 * m_Zoom, 32 + 16 * m_Zoom), !entry.IsDirectory,
                         texSize.Width, texSize.Height)) {
                std::error_code ec;
                bool isDir = std::filesystem::is_directory(entry.Path, ec);

                if (ImGui::IsMouseDoubleClicked(0)) {
                    if (isDir) {
                        SetDirectory(entry.Path);
                        break;
                    } else
                        Finalize(filename);
                } else {
                    if ((isDir && m_Type == IFD_DIALOG_DIRECTORY) || !isDir)
                        Select(entry.Path, ImGui::GetIO().KeyCtrl);
                }
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) m_SelectedFileItem = fileId;
            fileId++;
        }
    }
}

void UFileDialog::RenderPopups() {
    bool openAreYouSureDlg = false, openNewFileDlg = false, openNewDirectoryDlg = false;
    if (ImGui::BeginPopupContextItem("##dir_context")) {
        if (ImGui::Selectable("New file")) openNewFileDlg = true;
        if (ImGui::Selectable("New directory")) openNewDirectoryDlg = true;
        if (m_SelectedFileItem != -1 && ImGui::Selectable("Delete")) openAreYouSureDlg = true;
        ImGui::EndPopup();
    }
    if (openAreYouSureDlg) ImGui::OpenPopup("Are you sure?##delete");
    if (openNewFileDlg) ImGui::OpenPopup("Enter file name##newfile");
    if (openNewDirectoryDlg) ImGui::OpenPopup("Enter directory name##newdir");
    if (ImGui::BeginPopupModal("Are you sure?##delete")) {
        if (m_SelectedFileItem >= static_cast<int>(m_Content.size()) || m_Content.size() == 0)
            ImGui::CloseCurrentPopup();
        else {
            const FileData& data = m_Content[m_SelectedFileItem];
            ImGui::TextWrapped("Are you sure you want to delete %s?",
                               data.Path.filename().string().c_str());
            if (ImGui::Button("Yes")) {
                std::error_code ec;
                std::filesystem::remove_all(data.Path, ec);
                SetDirectory(m_CurrentDirectory, false); // refresh
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No")) ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Enter file name##newfile")) {
        ImGui::PushItemWidth(250.0f);
        ImGui::InputText("##newfilename", m_NewEntryBuffer,
                         1024); // TODO: remove hardcoded literals
        ImGui::PopItemWidth();

        if (ImGui::Button("OK")) {
            std::ofstream out((m_CurrentDirectory / std::string(m_NewEntryBuffer)).string());
            out << "";
            out.close();

            SetDirectory(m_CurrentDirectory, false); // refresh
            m_NewEntryBuffer[0] = 0;

            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_NewEntryBuffer[0] = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Enter directory name##newdir")) {
        ImGui::PushItemWidth(250.0f);
        ImGui::InputText("##newfilename", m_NewEntryBuffer,
                         1024); // TODO: remove hardcoded literals
        ImGui::PopItemWidth();

        if (ImGui::Button("OK")) {
            std::error_code ec;
            std::filesystem::create_directory(m_CurrentDirectory / std::string(m_NewEntryBuffer),
                                              ec);
            SetDirectory(m_CurrentDirectory, false); // refresh
            m_NewEntryBuffer[0] = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            m_NewEntryBuffer[0] = 0;
        }
        ImGui::EndPopup();
    }
}

void UFileDialog::RenderFileDialog() {
    /***** TOP BAR *****/
    bool noBackHistory = m_BackHistory.empty(), noForwardHistory = m_ForwardHistory.empty();

    ImGui::PushStyleColor(ImGuiCol_Button, 0);
    if (noBackHistory) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    if (ImGui::ArrowButtonEx("##back", ImGuiDir_Left, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE),
                             m_BackHistory.empty() * ImGuiItemFlags_Disabled)) {
        std::filesystem::path newPath = m_BackHistory.top();
        m_BackHistory.pop();
        m_ForwardHistory.push(m_CurrentDirectory);

        SetDirectory(newPath, false);
    }
    if (noBackHistory) ImGui::PopStyleVar();
    ImGui::SameLine();

    if (noForwardHistory) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    if (ImGui::ArrowButtonEx("##forward", ImGuiDir_Right,
                             ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE),
                             m_ForwardHistory.empty() * ImGuiItemFlags_Disabled)) {
        std::filesystem::path newPath = m_ForwardHistory.top();
        m_ForwardHistory.pop();
        m_BackHistory.push(m_CurrentDirectory);

        SetDirectory(newPath, false);
    }
    if (noForwardHistory) ImGui::PopStyleVar();
    ImGui::SameLine();

    if (ImGui::ArrowButtonEx("##up", ImGuiDir_Up, ImVec2(GUI_ELEMENT_SIZE, GUI_ELEMENT_SIZE))) {
        if (m_CurrentDirectory.has_parent_path()) SetDirectory(m_CurrentDirectory.parent_path());
    }

    std::filesystem::path curDirCopy = m_CurrentDirectory;
    if (PathBox("##pathbox", curDirCopy, m_PathBuffer, ImVec2(-250, GUI_ELEMENT_SIZE)))
        SetDirectory(curDirCopy);
    ImGui::SameLine();

    if (FavoriteButton("##dirfav", std::count(m_Favorites.begin(), m_Favorites.end(),
                                              m_CurrentDirectory.string()))) {
        if (std::count(m_Favorites.begin(), m_Favorites.end(), m_CurrentDirectory.string()))
            RemoveFavorite(m_CurrentDirectory.string());
        else
            AddFavorite(m_CurrentDirectory.string());
    }
    ImGui::SameLine();
    ImGui::PopStyleColor();

    if (ImGui::InputTextEx("##searchTB", "Search", m_SearchBuffer, 128,
                           ImVec2(-FLT_MIN, GUI_ELEMENT_SIZE), 0)) // TODO: no hardcoded literals
        SetDirectory(m_CurrentDirectory, false);                   // refresh

    /***** CONTENT *****/
    float bottomBarHeight = (GImGui->FontSize + ImGui::GetStyle().FramePadding.y +
                             ImGui::GetStyle().ItemSpacing.y * 2.0f) *
                            2;
    if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_Resizable, ImVec2(0, -bottomBarHeight))) {
        ImGui::TableSetupColumn("##tree", ImGuiTableColumnFlags_WidthFixed, 125.0f);
        ImGui::TableSetupColumn("##content", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();

        // the tree on the left side
        ImGui::TableSetColumnIndex(0);
        ImGui::BeginChild("##treeContainer", ImVec2(0, -bottomBarHeight));
        for (auto node : m_TreeCache)
            RenderTree(node);
        ImGui::EndChild();

        // content on the right side
        ImGui::TableSetColumnIndex(1);
        ImGui::BeginChild("##contentContainer", ImVec2(0, -bottomBarHeight));
        RenderContent();
        ImGui::EndChild();
        if (ImGui::IsItemHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f) {
            m_Zoom =
                std::min<float>(25.0f, std::max<float>(1.0f, m_Zoom + ImGui::GetIO().MouseWheel));
            RefreshThumbnails();
            if (!s_WrappedFileNameMap.empty()) s_WrappedFileNameMap.clear();
        }

        // New file, New directory and Delete popups
        RenderPopups();

        ImGui::EndTable();
    }

    /***** BOTTOM BAR *****/
    ImGui::Text("File name:");
    ImGui::SameLine();
    if (ImGui::InputTextEx("##file_input", "Filename", m_InputTextbox, 1024,
                           ImVec2((m_Type != IFD_DIALOG_DIRECTORY) ? -250.0f : -FLT_MIN, 0),
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
        bool success = Finalize(std::string(m_InputTextbox));
#ifdef CZ_PLATFORM_WINDOWS
        if (!success) MessageBeep(MB_ICONERROR);
#else
        (void)success;
#endif
    }
    if (m_Type != IFD_DIALOG_DIRECTORY) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        int sel = static_cast<int>(m_FilterSelection);
        if (ImGui::Combo("##ext_combo", &sel, m_Filter.c_str())) {
            m_FilterSelection = static_cast<size_t>(sel);
            SetDirectory(m_CurrentDirectory, false); // refresh
        }
    }

    // buttons
    float ok_cancel_width = GUI_ELEMENT_SIZE * 7;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ok_cancel_width);
    if (ImGui::Button(m_Type == IFD_DIALOG_SAVE ? "Save" : "Open",
                      ImVec2(ok_cancel_width / 2 - ImGui::GetStyle().ItemSpacing.x, 0.0f))) {
        std::string filename(m_InputTextbox);
        bool success = false;
        if (!filename.empty() || m_Type == IFD_DIALOG_DIRECTORY) success = Finalize(filename);
#ifdef CZ_PLATFORM_WINDOWS
        if (!success) MessageBeep(MB_ICONERROR);
#else
        (void)success;
#endif
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(-FLT_MIN, 0.0f))) {
        if (m_Type == IFD_DIALOG_DIRECTORY)
            m_IsOpen = false;
        else
            Finalize();
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape))
        m_IsOpen = false;
}
