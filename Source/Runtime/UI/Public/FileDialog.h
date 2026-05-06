#pragma once

#include "CoreMinimal.h"
#include "UIExport.h"

#include "FileTreeNode.h"
#include "FileUtils.h"
#include "Texture.h"
#include "ThreadPool.h"

#include <stack>

DECLARE_LOG_CATEGORY_EXTERN(LogFileDialog, Info);

#define IFD_DIALOG_FILE      0
#define IFD_DIALOG_DIRECTORY 1
#define IFD_DIALOG_SAVE      2

class FileData {
public:
    FileData(const std::filesystem::path& path);

    std::filesystem::path Path;
    bool IsDirectory;
    size_t Size;
    time_t DateModified;

    TRef<CTexture> Thumbnail;
    int ThumbnailWidth, ThumbnailHeight;
};

class UI_API UFileDialog {
public:
    static UFileDialog& Get();

    UFileDialog();
    ~UFileDialog();

    bool Save(const std::string& key, const std::string& title, const std::string& filter,
              const std::string& startingDir = "");

    bool Open(const std::string& key, const std::string& title, const std::string& filter,
              bool isMultiselect = false, const std::string& startingDir = "");

    bool IsDone(const std::string& key);

    inline bool HasResult() { return m_Result.size(); }
    inline const std::filesystem::path& GetResult() { return m_Result[0]; }
    inline const std::vector<std::filesystem::path>& GetResults() { return m_Result; }

    void Close();
    void Shutdown();

    void RemoveFavorite(const std::string& path);
    void AddFavorite(const std::string& path);
    inline const std::vector<std::string>& GetFavorites() { return m_Favorites; }

    inline void SetZoom(float z) {
        m_Zoom = std::min<float>(25.0f, std::max<float>(1.0f, z));
        RefreshThumbnails();
    }
    inline float GetZoom() { return m_Zoom; }

private:
    TRef<CTexture> CreateTexture(uint8_t* data, int w, int h,
                                 char fmt); // char -> fmt -> { 0 = BGRA, 1 = RGBA }

    void Select(const std::filesystem::path& path, bool isCtrlDown = false);

    bool Finalize(const std::string& filename = "");

    void ParseFilter(const std::string& filter);

    TRef<CTexture> GetIcon(const std::filesystem::path& path);
    FRawFileImage GetDefaultIcon(const std::filesystem::path& path);

    TRef<CTexture> GetThumbnail(const std::filesystem::path& path);

    void RequestThumbnails();
    void ProcessPendingThumbs();
    void RefreshThumbnails();
    void ClearThumbnails();

    void ClearTree(FileTreeNode* node);
    void RenderTree(FileTreeNode* node);

    void SetDirectory(const std::filesystem::path& p, bool addHistory = true);
    void SortContent(unsigned int column, unsigned int sortDirection);
    void RenderContent();

    void RenderPopups();
    void RenderFileDialog();

private:
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

    CThreadPool m_ThumbPool{ 4 };
    std::mutex m_ThumbMutex;

    std::unordered_map<std::string, TRef<CTexture>> m_ThumbMap;
    std::vector<FRawFileImage> m_PendingRawThumbs;
    std::vector<TRef<CTexture>> m_TextureGarbage;

    std::thread* m_ThumbnailLoader;
    bool m_ThumbnailLoaderRunning;

    std::vector<FileTreeNode*> m_TreeCache;

    unsigned int m_SortColumn;
    unsigned int m_SortDirection;
    std::vector<FileData> m_Content;
};
