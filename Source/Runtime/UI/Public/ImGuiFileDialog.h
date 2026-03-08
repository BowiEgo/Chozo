#pragma once

#include "CoreMinimal.h"
#include "FileUtils.h"
#include "RHIContext.h"
#include "RHITexture2D.h"
#include "UIExport.h"

#include <algorithm> // std::min, std::max
#include <ctime>
#include <filesystem>
#include <functional>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

DECLARE_LOG_CATEGORY_EXTERN(LogImGuiFileDialog, Info);

#define IFD_DIALOG_FILE 0
#define IFD_DIALOG_DIRECTORY 1
#define IFD_DIALOG_SAVE 2

class FileTreeNode {
public:
#ifdef CZ_PLATFORM_WINDOWS
    FileTreeNode(const std::wstring& path) {
        Path = std::filesystem::path(path);
        Read = false;
    }
#endif

    FileTreeNode(const std::string& path) {
        Path = std::filesystem::path(path);
        Read = false;
    }

    std::filesystem::path Path;
    bool Read;
    std::vector<FileTreeNode*> Children;
};

class FileData {
public:
    FileData(const std::filesystem::path& path);

    std::filesystem::path Path;
    bool IsDirectory;
    size_t Size;
    time_t DateModified;

    bool HasIconPreview;
    TRef<IRHITexture2D> IconPreview;
    uint8_t* IconPreviewData;
    int IconPreviewWidth, IconPreviewHeight;
};

class UI_API ImGuiFileDialog {
public:
    static inline ImGuiFileDialog& Get(IRHIContext* context) {
        static ImGuiFileDialog ret(context);
        return ret;
    }

    ImGuiFileDialog(IRHIContext* context);
    ~ImGuiFileDialog();

    bool Save(const std::string& key, const std::string& title, const std::string& filter,
              const std::string& startingDir = "");

    bool Open(const std::string& key, const std::string& title, const std::string& filter,
              bool isMultiselect = false, const std::string& startingDir = "");

    bool IsDone(const std::string& key);

    inline bool HasResult() { return m_Result.size(); }
    inline const std::filesystem::path& GetResult() { return m_Result[0]; }
    inline const std::vector<std::filesystem::path>& GetResults() { return m_Result; }

    void Close();

    void RemoveFavorite(const std::string& path);
    void AddFavorite(const std::string& path);
    inline const std::vector<std::string>& GetFavorites() { return m_Favorites; }

    inline void SetZoom(float z) {
        m_Zoom = std::min<float>(25.0f, std::max<float>(1.0f, z));
        RefreshIconPreview();
    }
    inline float GetZoom() { return m_Zoom; }

private:
    TRef<IRHITexture2D> CreateTexture(uint8_t* data, int w, int h,
                                      char fmt); // char -> fmt -> { 0 = BGRA, 1 = RGBA }

    void Select(const std::filesystem::path& path, bool isCtrlDown = false);

    bool Finalize(const std::string& filename = "");

    void ParseFilter(const std::string& filter);

    TRef<IRHITexture2D> GetIcon(const std::filesystem::path& path);
    FRawIcon GetDefaultIcon(const std::filesystem::path& path);

    void ClearIcons();
    void RefreshIconPreview();
    void ClearIconPreview();

    void StopPreviewLoader();
    void LoadPreview();

    void ClearTree(FileTreeNode* node);
    void RenderTree(FileTreeNode* node);

    void SetDirectory(const std::filesystem::path& p, bool addHistory = true);
    void SortContent(unsigned int column, unsigned int sortDirection);
    void RenderContent();

    void RenderPopups();
    void RenderFileDialog();

private:
    IRHIContext* m_GraphicContext;

    std::string m_CurrentKey;
    std::string m_CurrentTitle;
    std::filesystem::path m_CurrentDirectory;
    bool m_IsMultiselect;
    bool m_IsOpen;
    uint8_t m_Type;
    char m_InputTextbox[1024];
    char m_PathBuffer[1024];
    char m_NewEntryBuffer[1024];
    char m_SearchBuffer[128];
    std::vector<std::string> m_Favorites;
    bool m_CalledOpenPopup;
    std::stack<std::filesystem::path> m_BackHistory, m_ForwardHistory;
    float m_Zoom;

    std::vector<std::filesystem::path> m_Selections;
    int m_SelectedFileItem;

    std::vector<std::filesystem::path> m_Result;

    std::string m_Filter;
    std::vector<std::vector<std::string>> m_FilterExtensions;
    size_t m_FilterSelection;

    std::vector<int> m_IconIndices;
    std::vector<std::string> m_IconFilepaths; // m_IconIndices[x] <-> m_IconFilepaths[x]
    std::unordered_map<std::string, TRef<IRHITexture2D>> m_Icons;
    std::vector<TRef<IRHITexture2D>> m_IconsGarbage;

    std::thread* m_PreviewLoader;
    bool m_PreviewLoaderRunning;

    std::vector<FileTreeNode*> m_TreeCache;

    unsigned int m_SortColumn;
    unsigned int m_SortDirection;
    std::vector<FileData> m_Content;
};
