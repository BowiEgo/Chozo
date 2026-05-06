#pragma once

#include "CoreMinimal.h"
#include "UIExport.h"

#include <filesystem>

class UI_API FileTreeNode {
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
