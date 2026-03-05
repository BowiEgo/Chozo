#include "ImGuiFileDialog.h"

#include "RHIAPI.h"

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
    #define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef CZ_PLATFORM_WINDOWS
    #include <windows.h> // MUST be included BEFORE shellapi.h

    #include <shellapi.h>

    #include <lmcons.h>
    #pragma comment(lib, "Shell32.lib")
#else
    #include <pwd.h>
    #include <unistd.h>
#endif

#include "imgui_internal.h"
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define ICON_SIZE (ImGui::GetFontSize() + 3)
#define GUI_ELEMENT_SIZE ((std::max)(ImGui::GetFontSize() + 10.f, 24.f))
#define DEFAULT_ICON_SIZE 32
#define PI 3.141592f

DEFINE_LOG_CATEGORY(LogImGuiFileDialog);

static const char* GetDefaultFolderIcon();
static const char* GetDefaultFileIcon();

bool IsHiddenOrSystem(const std::filesystem::path& p) {
#ifdef CZ_PLATFORM_WINDOWS
    DWORD attributes = GetFileAttributesW(p.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        return (attributes & FILE_ATTRIBUTE_HIDDEN) || (attributes & FILE_ATTRIBUTE_SYSTEM);
    }
#else
    if (filename.size() > 0 && filename[0] == '.') return true;
#endif
    return false;
}

/* UI CONTROLS */
bool FolderNode(const char* label, ImTextureID icon, bool& clicked) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    clicked = false;

    ImU32 id = window->GetID(label);
    int opened = window->StateStorage.GetInt(id, 0);
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
                path = std::filesystem::u8path(newPath);
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
            if (std::filesystem::exists(tempStr)) path = std::filesystem::u8path(tempStr);
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
    float innerRadius = size / 4;
    float outerRadius = size / 2;
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
    ImVec2 textSize = ImGui::CalcTextSize(label, 0, true, size.x);

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

    window->DrawList->AddText(
        g.Font, g.FontSize, ImVec2(pos.x + (size.x - textSize.x) / 2.0f, pos.y + iconSize),
        ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), label, 0, size.x);

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

    HasIconPreview = false;
    IconPreview = nullptr;
    IconPreviewData = nullptr;
    IconPreviewHeight = 0;
    IconPreviewWidth = 0;
}

ImGuiFileDialog::ImGuiFileDialog(IRHIContext* context) : m_GraphicContext(context) {
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

    m_PreviewLoader = nullptr;
    m_PreviewLoaderRunning = false;

    SetDirectory(std::filesystem::current_path(), false);

    // favorites are available on every OS
    FileTreeNode* quickAccess = new FileTreeNode("Quick Access");
    quickAccess->Read = true;
    m_TreeCache.push_back(quickAccess);

#ifdef CZ_PLATFORM_WINDOWS
    wchar_t username[UNLEN + 1] = { 0 };
    DWORD username_len = UNLEN + 1;
    GetUserNameW(username, &username_len);

    std::wstring userPath = L"C:\\Users\\" + std::wstring(username) + L"\\";

    // Quick Access / Bookmarks
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Documents"));
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
    quickAccess->Children.push_back(new FileTreeNode(userPath + L"Pictures"));

    // OneDrive
    FileTreeNode* oneDrive = new FileTreeNode(userPath + L"OneDrive");
    m_TreeCache.push_back(oneDrive);

    // This PC
    FileTreeNode* thisPC = new FileTreeNode("This PC");
    thisPC->Read = true;
    if (std::filesystem::exists(userPath + L"3D Objects"))
        thisPC->Children.push_back(new FileTreeNode(userPath + L"3D Objects"));
    thisPC->Children.push_back(new FileTreeNode(userPath + L"Desktop"));
    thisPC->Children.push_back(new FileTreeNode(userPath + L"Documents"));
    thisPC->Children.push_back(new FileTreeNode(userPath + L"Downloads"));
    thisPC->Children.push_back(new FileTreeNode(userPath + L"Music"));
    thisPC->Children.push_back(new FileTreeNode(userPath + L"Pictures"));
    thisPC->Children.push_back(new FileTreeNode(userPath + L"Videos"));
    DWORD d = GetLogicalDrives();
    for (int i = 0; i < 26; i++)
        if (d & (1 << i))
            thisPC->Children.push_back(new FileTreeNode(std::string(1, 'A' + i) + ":"));
    m_TreeCache.push_back(thisPC);
#else
    std::error_code ec;

    // Quick Access
    struct passwd* pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);
    if (pw) {
        std::string homePath = "/home/" + std::string(pw->pw_name);

        if (std::filesystem::exists(homePath, ec))
            quickAccess->Children.push_back(new FileTreeNode(homePath));
        if (std::filesystem::exists(homePath + "/Desktop", ec))
            quickAccess->Children.push_back(new FileTreeNode(homePath + "/Desktop"));
        if (std::filesystem::exists(homePath + "/Documents", ec))
            quickAccess->Children.push_back(new FileTreeNode(homePath + "/Documents"));
        if (std::filesystem::exists(homePath + "/Downloads", ec))
            quickAccess->Children.push_back(new FileTreeNode(homePath + "/Downloads"));
        if (std::filesystem::exists(homePath + "/Pictures", ec))
            quickAccess->Children.push_back(new FileTreeNode(homePath + "/Pictures"));
    }

    // This PC
    FileTreeNode* thisPC = new FileTreeNode("This PC");
    thisPC->Read = true;
    for (const auto& entry : std::filesystem::directory_iterator("/", ec)) {
        if (std::filesystem::is_directory(entry, ec))
            thisPC->Children.push_back(new FileTreeNode(entry.path().string()));
    }
    m_TreeCache.push_back(thisPC);
#endif
}

ImGuiFileDialog::~ImGuiFileDialog() {
    ClearIconPreview();
    ClearIcons();
    m_IconsGarbage.clear();

    for (auto fn : m_TreeCache)
        ClearTree(fn);
    m_TreeCache.clear();
}

bool ImGuiFileDialog::Save(const std::string& key, const std::string& title,
                           const std::string& filter, const std::string& startingDir) {
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
        SetDirectory(std::filesystem::u8path(startingDir), false);
    else
        SetDirectory(m_CurrentDirectory, false); // refresh contents

    return true;
}

bool ImGuiFileDialog::Open(const std::string& key, const std::string& title,
                           const std::string& filter, bool isMultiselect,
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
    m_IsMultiselect = isMultiselect;
    m_Type = filter.empty() ? IFD_DIALOG_DIRECTORY : IFD_DIALOG_FILE;

    ParseFilter(filter);
    if (!startingDir.empty())
        SetDirectory(std::filesystem::u8path(startingDir), false);
    else
        SetDirectory(m_CurrentDirectory, false); // refresh contents

    return true;
}

bool ImGuiFileDialog::IsDone(const std::string& key) {
    bool isMe = m_CurrentKey == key;

    if (!m_IconsGarbage.empty()) {
        const int MAX_CLEANUP_PER_FRAME = 10;
        int cleanedCount = 0;

        auto it = m_IconsGarbage.begin();
        while (it != m_IconsGarbage.end() && cleanedCount < MAX_CLEANUP_PER_FRAME) {
            it = m_IconsGarbage.erase(it);
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

void ImGuiFileDialog::Close() {
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

    // free icon textures
    ClearIconPreview();
    ClearIcons();
}

void ImGuiFileDialog::RemoveFavorite(const std::string& path) {
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

void ImGuiFileDialog::AddFavorite(const std::string& path) {
    if (std::count(m_Favorites.begin(), m_Favorites.end(), path) > 0) return;

    if (!std::filesystem::exists(std::filesystem::u8path(path))) return;

    m_Favorites.push_back(path);

    // add to sidebar
    for (auto& p : m_TreeCache)
        if (p->Path == "Quick Access") {
            p->Children.push_back(new FileTreeNode(path));
            break;
        }
}

TRef<IRHITexture2D> ImGuiFileDialog::CreateTexture(uint8_t* data, int w, int h, char fmt) {
    FTexture2DSpecification spec;
    spec.Name = "ImGuiFileDialog_Preview";
    spec.Size = { (uint32_t)w, (uint32_t)h };
    spec.Format = (fmt == 0) ? EPixelFormat::BGRA8_UNORM : EPixelFormat::RGBA8_UNORM;
    spec.Usage = ETextureUsage::Texture;
    spec.bGenerateMips = true;

    TRef<IRHITexture2D> texture = IRHIAPI::CreateTexture2D(m_GraphicContext, spec);

    FBuffer imageData(data, w * h * 4);
    texture->SetData(imageData);

    return texture;
}

void ImGuiFileDialog::Select(const std::filesystem::path& path, bool isCtrlDown) {
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

bool ImGuiFileDialog::Finalize(const std::string& filename) {
    bool hasResult =
        (!filename.empty() && m_Type != IFD_DIALOG_DIRECTORY) || m_Type == IFD_DIALOG_DIRECTORY;

    if (hasResult) {
        if (!m_IsMultiselect || m_Selections.size() <= 1) {
            std::filesystem::path path = std::filesystem::u8path(filename);
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

void ImGuiFileDialog::ParseFilter(const std::string& filter) {
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

TRef<IRHITexture2D> ImGuiFileDialog::GetIcon(const std::filesystem::path& path) {
    std::string pathU8 = path.string();
    if (m_Icons.count(pathU8) > 0) return m_Icons[pathU8];

    std::error_code ec;
    m_Icons[pathU8] = nullptr;

#ifdef CZ_PLATFORM_WINDOWS
    std::wstring pathW = path.wstring();
    // if (std::filesystem::is_directory(path) && !pathW.empty() && pathW.back() != L'\\') {
    //     pathW += L'\\';
    // } else {
    std::replace(pathW.begin(), pathW.end(), L'/', L'\\');
    // }

    SHFILEINFOW fileInfo = { 0 };
    UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
    DWORD attrs = FILE_ATTRIBUTE_NORMAL;

    if (!std::filesystem::exists(path, ec)) {
        flags |= SHGFI_USEFILEATTRIBUTES;
        attrs = FILE_ATTRIBUTE_DIRECTORY;
    }

    // if (std::filesystem::is_directory(path, ec)) {
    //     attrs = FILE_ATTRIBUTE_DIRECTORY;
    // }

    // if (!std::filesystem::exists(path, ec)) {
    //     flags |= SHGFI_USEFILEATTRIBUTES;
    // }

    if (SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags)) {
        std::string narrowPath = std::filesystem::path(pathW).string();
        CZ_LOG(LogImGuiFileDialog, Trace, "File: {}, Icon Index: {}", narrowPath, fileInfo.iIcon);

        auto itr = std::find(m_IconIndices.begin(), m_IconIndices.end(), fileInfo.iIcon);
        if (itr != m_IconIndices.end()) {
            const std::string& existingPath = m_IconFilepaths[itr - m_IconIndices.begin()];
            m_Icons[pathU8] = m_Icons[existingPath];
            if (fileInfo.hIcon) DestroyIcon(fileInfo.hIcon);
            return m_Icons[pathU8];
        }

        ICONINFO iconInfo = { 0 };
        if (GetIconInfo(fileInfo.hIcon, &iconInfo)) {
            if (iconInfo.hbmColor) {
                DIBSECTION ds;
                GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
                int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

                if (byteSize > 0) {
                    uint8_t* data = (uint8_t*)malloc(byteSize);
                    GetBitmapBits(iconInfo.hbmColor, byteSize, data);
                    m_Icons[pathU8] =
                        this->CreateTexture(data, ds.dsBm.bmWidth, ds.dsBm.bmHeight, 0);
                    free(data);

                    m_IconIndices.push_back(fileInfo.iIcon);
                    m_IconFilepaths.push_back(pathU8);
                }
                DeleteObject(iconInfo.hbmColor);
            }
            if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
        }
        if (fileInfo.hIcon) DestroyIcon(fileInfo.hIcon);
    }

    return m_Icons[pathU8];

    // DWORD attrs = 0;
    // UINT flags = SHGFI_ICON | SHGFI_LARGEICON;
    // if (!std::filesystem::exists(path, ec)) {
    //     flags |= SHGFI_USEFILEATTRIBUTES;
    //     attrs = FILE_ATTRIBUTE_DIRECTORY;
    // }

    // SHFILEINFOW fileInfo = { 0 };
    // std::wstring pathW = path.wstring();
    // for (int i = 0; i < pathW.size(); i++)
    //     if (pathW[i] == '/') pathW[i] = '\\';
    // SHGetFileInfoW(pathW.c_str(), attrs, &fileInfo, sizeof(SHFILEINFOW), flags);

    // if (fileInfo.hIcon == nullptr) return nullptr;

    // // check if icon is already loaded
    // auto itr = std::find(m_IconIndices.begin(), m_IconIndices.end(), fileInfo.iIcon);
    // if (itr != m_IconIndices.end()) {
    //     const std::string& existingIconFilepath = m_IconFilepaths[itr - m_IconIndices.begin()];
    //     m_Icons[pathU8] = m_Icons[existingIconFilepath];
    //     return m_Icons[pathU8];
    // }

    // m_IconIndices.push_back(fileInfo.iIcon);
    // m_IconFilepaths.push_back(pathU8);

    // ICONINFO iconInfo = { 0 };
    // GetIconInfo(fileInfo.hIcon, &iconInfo);

    // if (iconInfo.hbmColor == nullptr) return nullptr;

    // DIBSECTION ds;
    // GetObject(iconInfo.hbmColor, sizeof(ds), &ds);
    // int byteSize = ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8);

    // if (byteSize == 0) return nullptr;

    // uint8_t* data = (uint8_t*)malloc(byteSize);
    // GetBitmapBits(iconInfo.hbmColor, byteSize, data);

    // m_Icons[pathU8] = this->CreateTexture(data, ds.dsBm.bmWidth, ds.dsBm.bmHeight, 0);

    // free(data);

    // DeleteObject(iconInfo.hbmColor);
    // if (iconInfo.hbmMask) {
    //     DeleteObject(iconInfo.hbmMask);
    // }
    // DestroyIcon(fileInfo.hIcon);

    // return m_Icons[pathU8];
#else
    int iconID = 1;
    if (std::filesystem::is_directory(path, ec)) iconID = 0;

    // check if icon is already loaded
    auto itr = std::find(m_IconIndices.begin(), m_IconIndices.end(), iconID);
    if (itr != m_IconIndices.end()) {
        const std::string& existingIconFilepath = m_IconFilepaths[itr - m_IconIndices.begin()];
        m_Icons[pathU8] = m_Icons[existingIconFilepath];
        return m_Icons[pathU8];
    }

    m_IconIndices.push_back(iconID);
    m_IconFilepaths.push_back(pathU8);

    ImVec4 wndBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

    // light theme - load default icons
    if ((wndBg.x + wndBg.y + wndBg.z) / 3.0f > 0.5f) {
        uint8_t* data = (uint8_t*)GetDefaultFileIcon();
        if (iconID == 0) data = (uint8_t*)GetDefaultFolderIcon();
        m_Icons[pathU8] = this->CreateTexture(data, DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE, 0);
    }
    // dark theme - invert the colors
    else {
        uint8_t* data = (uint8_t*)GetDefaultFileIcon();
        if (iconID == 0) data = (uint8_t*)GetDefaultFolderIcon();

        uint8_t* invData = (uint8_t*)malloc(DEFAULT_ICON_SIZE * DEFAULT_ICON_SIZE * 4);
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                int index = (y * DEFAULT_ICON_SIZE + x) * 4;
                invData[index + 0] = 255 - data[index + 0];
                invData[index + 1] = 255 - data[index + 1];
                invData[index + 2] = 255 - data[index + 2];
                invData[index + 3] = data[index + 3];
            }
        }
        m_Icons[pathU8] = this->CreateTexture(invData, DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE, 0);

        free(invData);
    }

    return m_Icons[pathU8];
#endif
}

void ImGuiFileDialog::ClearIcons() {
    m_IconFilepaths.clear();
    m_IconIndices.clear();
    for (auto& [path, icon] : m_Icons)
        m_IconsGarbage.push_back(icon);

    m_Icons.clear();
}

void ImGuiFileDialog::RefreshIconPreview() {
    if (m_Zoom >= 5.0f) {
        if (m_PreviewLoader == nullptr) {
            m_PreviewLoaderRunning = true;
            m_PreviewLoader = new std::thread(&ImGuiFileDialog::LoadPreview, this);
        }
    } else
        ClearIconPreview();
}

void ImGuiFileDialog::ClearIconPreview() {
    StopPreviewLoader();

    for (auto& data : m_Content) {
        if (!data.HasIconPreview) continue;

        data.HasIconPreview = false;
        m_IconsGarbage.push_back(data.IconPreview);
        data.IconPreview = nullptr;

        if (data.IconPreviewData != nullptr) {
            stbi_image_free(data.IconPreviewData);
            data.IconPreviewData = nullptr;
        }
    }
}

void ImGuiFileDialog::StopPreviewLoader() {
    if (m_PreviewLoader != nullptr) {
        m_PreviewLoaderRunning = false;

        if (m_PreviewLoader && m_PreviewLoader->joinable()) m_PreviewLoader->join();

        delete m_PreviewLoader;
        m_PreviewLoader = nullptr;
    }
}

void ImGuiFileDialog::LoadPreview() {
    CZ_LOG(LogImGuiFileDialog, Trace, "LoadPreview {}", m_PreviewLoaderRunning);
    for (size_t i = 0; m_PreviewLoaderRunning && i < m_Content.size(); i++) {
        auto& data = m_Content[i];

        if (data.HasIconPreview) continue;

        if (data.Path.has_extension()) {
            std::string ext = data.Path.extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" ||
                ext == ".tga") {
                int width, height, nrChannels;
                unsigned char* imageData = stbi_load(data.Path.string().c_str(), &width, &height,
                                                     &nrChannels, STBI_rgb_alpha);

                if (imageData == nullptr || width == 0 || height == 0) continue;

                CZ_LOG(LogImGuiFileDialog, Trace, "LoadPreview:CreateTexture");
                TRef<IRHITexture2D> icon = CreateTexture(imageData, width, height, 1);
                CZ_LOG(LogImGuiFileDialog, Trace, "LoadPreview:Created");

                data.HasIconPreview = true;
                data.IconPreview = icon;
                data.IconPreviewData = imageData;
                data.IconPreviewWidth = width;
                data.IconPreviewHeight = height;
            }
        }
    }

    m_PreviewLoaderRunning = false;
}

void ImGuiFileDialog::ClearTree(FileTreeNode* node) {
    if (node == nullptr) return;

    for (auto n : node->Children)
        ClearTree(n);

    delete node;
    node = nullptr;
}

void ImGuiFileDialog::SetDirectory(const std::filesystem::path& p, bool addHistory) {
    bool isSameDir = m_CurrentDirectory == p;

    if (addHistory && !isSameDir) m_BackHistory.push(m_CurrentDirectory);

    m_CurrentDirectory = p;
#ifdef CZ_PLATFORM_WINDOWS
    // drives don't work well without the backslash symbol
    if (p.string().size() == 2 && p.string()[1] == ':')
        m_CurrentDirectory = std::filesystem::u8path(p.string() + "\\");
#endif

    ClearIconPreview();
    m_Content.clear(); // p == "" after this line, due to reference
    m_SelectedFileItem = -1;

    if (m_Type == IFD_DIALOG_DIRECTORY || m_Type == IFD_DIALOG_FILE) m_InputTextbox[0] = 0;
    m_Selections.clear();

    if (!isSameDir) {
        m_SearchBuffer[0] = 0;
        ClearIcons();
    }

    if (p.string() == "Quick Access") {
        for (auto& node : m_TreeCache) {
            if (node->Path == p)
                for (auto& c : node->Children)
                    m_Content.push_back(FileData(c->Path));
        }
    } else if (p.string() == "This PC") {
        for (auto& node : m_TreeCache) {
            if (node->Path == p)
                for (auto& c : node->Children)
                    m_Content.push_back(FileData(c->Path));
        }
    } else {
        std::error_code ec;
        if (std::filesystem::exists(m_CurrentDirectory, ec))
            for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory, ec)) {
                if (IsHiddenOrSystem(entry.path())) continue;
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
    RefreshIconPreview();
}

void ImGuiFileDialog::SortContent(unsigned int column, unsigned int sortDirection) {
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

void ImGuiFileDialog::RenderTree(FileTreeNode* node) {
    // directory
    std::error_code ec;
    ImGui::PushID(node);
    bool isClicked = false;
    std::string displayName = node->Path.stem().string();
    if (displayName.size() == 0) displayName = node->Path.string();
    if (FolderNode(displayName.c_str(), (ImTextureID)GetIcon(node->Path)->GetDescriptorSet(),
                   isClicked)) {
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
            RenderTree(c);

        ImGui::TreePop();
    }
    if (isClicked) SetDirectory(node->Path);
    ImGui::PopID();
}

void ImGuiFileDialog::RenderContent() {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) m_SelectedFileItem = -1;

    // table view
    if (m_Zoom == 1.0f) {
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

                ImGui::TableNextRow();

                // file name
                ImGui::TableSetColumnIndex(0);
                ImGui::Image((ImTextureID)GetIcon(entry.Path)->GetDescriptorSet(),
                             ImVec2(ICON_SIZE, ICON_SIZE));
                ImGui::SameLine();
                if (ImGui::Selectable(filename.c_str(), isSelected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowDoubleClick)) {
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

                // date
                ImGui::TableSetColumnIndex(1);
                auto tm = std::localtime(&entry.DateModified);
                if (tm != nullptr)
                    ImGui::Text("%d/%d/%d %02d:%02d", tm->tm_mon + 1, tm->tm_mday,
                                1900 + tm->tm_year, tm->tm_hour, tm->tm_min);
                else
                    ImGui::Text("---");

                // size
                ImGui::TableSetColumnIndex(2);
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
            if (entry.HasIconPreview && entry.IconPreviewData != nullptr) {
                entry.IconPreview = this->CreateTexture(
                    entry.IconPreviewData, entry.IconPreviewWidth, entry.IconPreviewHeight, 1);
                stbi_image_free(entry.IconPreviewData);
                entry.IconPreviewData = nullptr;
            }

            std::string filename = entry.Path.filename().string();
            if (filename.size() == 0) filename = entry.Path.string(); // drive

            bool isSelected = std::count(m_Selections.begin(), m_Selections.end(), entry.Path);

            if (FileIcon(filename.c_str(), isSelected,
                         entry.HasIconPreview
                             ? (ImTextureID)entry.IconPreview->GetDescriptorSet()
                             : (ImTextureID)GetIcon(entry.Path)->GetDescriptorSet(),
                         ImVec2(32 + 16 * m_Zoom, 32 + 16 * m_Zoom), entry.HasIconPreview,
                         entry.IconPreviewWidth, entry.IconPreviewHeight)) {
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

void ImGuiFileDialog::RenderPopups() {
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

void ImGuiFileDialog::RenderFileDialog() {
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
            RefreshIconPreview();
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

static const unsigned int file_icon[] = {
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x4c000000, 0xf5000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xdd000000, 0x2d000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0xd1000000, 0x6b000000, 0x6b000000,
    0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000,
    0x6b000000, 0x6a000000, 0xa1000000, 0xff000000, 0xff000000, 0x2e000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x54000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x46000000, 0xf5000000, 0xe0000000, 0xff000000, 0x30000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6a000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6e000000, 0xf8000000, 0x01000000, 0xc3000000, 0xff000000, 0x30000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00000000, 0x00000000, 0xd2000000, 0xff000000,
    0x30000000, 0x00000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x13000000, 0x00000000, 0x00000000, 0xd2000000,
    0xff000000, 0x30000000, 0x00000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x73000000, 0xff000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xbe000000, 0xff000000, 0x30000000, 0x00000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x65000000, 0xff000000, 0x34000000, 0x10000000, 0x10000000, 0x03000000,
    0x0a000000, 0xdb000000, 0xff000000, 0x2f000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0f000000, 0xd9000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xed000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x06000000, 0x5e000000, 0x6c000000, 0x6b000000, 0x6b000000,
    0x6b000000, 0x60000000, 0x9e000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x52000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6b000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6b000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x6a000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0x54000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x54000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0xff000000, 0xd2000000, 0x6b000000, 0x6b000000,
    0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000,
    0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000, 0x6b000000,
    0x6b000000, 0x6b000000, 0xd2000000, 0xff000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x4c000000, 0xf5000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xf5000000, 0x4b000000, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
};
static const unsigned int folder_icon[] = {
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00000000, 0x00000000, 0x45000000, 0x8a000000, 0x99000000, 0x97000000, 0x97000000, 0x97000000,
    0x97000000, 0x97000000, 0x98000000, 0x81000000, 0x35000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x9e000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0x80000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x76000000, 0xff000000, 0xff000000, 0xf6000000, 0xe2000000, 0xe2000000, 0xe2000000, 0xe2000000,
    0xe2000000, 0xe2000000, 0xe2000000, 0xff000000, 0xff000000, 0xff000000, 0x80000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xe7000000, 0xff000000, 0xbe000000, 0x11000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0xd1000000, 0xff000000, 0xff000000, 0x75000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xfa000000, 0xff000000, 0x5a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x06000000, 0xe0000000, 0xff000000, 0xff000000,
    0x68000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xf4000000, 0xff000000, 0x67000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x11000000, 0xe4000000, 0xff000000,
    0xff000000, 0xad000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000, 0x94000000,
    0x94000000, 0x94000000, 0x94000000, 0x96000000, 0x8b000000, 0x4f000000, 0x00000000, 0x00000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x17000000, 0xe8000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xaf000000, 0x00000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0e000000,
    0x88000000, 0xc3000000, 0xcd000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcc000000, 0xcc000000,
    0xcc000000, 0xcc000000, 0xcb000000, 0xcc000000, 0xe2000000, 0xff000000, 0xff000000, 0x81000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xb6000000, 0xff000000, 0xec000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x5b000000, 0xff000000, 0xf9000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x68000000, 0xff000000, 0xf4000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf3000000, 0xff000000, 0x6a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x6a000000, 0xff000000, 0xf3000000,
    0xf4000000, 0xff000000, 0x68000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x68000000, 0xff000000, 0xf4000000,
    0xfa000000, 0xff000000, 0x5a000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x5a000000, 0xff000000, 0xf9000000,
    0xea000000, 0xff000000, 0xb5000000, 0x05000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x05000000, 0xb5000000, 0xff000000, 0xea000000,
    0x7e000000, 0xff000000, 0xff000000, 0xeb000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000,
    0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000,
    0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000,
    0xd6000000, 0xd6000000, 0xd6000000, 0xd6000000, 0xeb000000, 0xff000000, 0xff000000, 0x7f000000,
    0x00000000, 0xac000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xac000000, 0x00000000,
    0x00000000, 0x00000000, 0x53000000, 0x8f000000, 0x9a000000, 0x99000000, 0x99000000, 0x99000000,
    0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000,
    0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000, 0x99000000,
    0x99000000, 0x99000000, 0x99000000, 0x9a000000, 0x8f000000, 0x53000000, 0x00000000, 0x00000000,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
    0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff, 0x00ffffff,
};
const char* GetDefaultFolderIcon() { return (const char*)&folder_icon[0]; }
const char* GetDefaultFileIcon() { return (const char*)&file_icon[0]; }
