#include "LinuxFile.h"

namespace ChozoUtils::File {

std::filesystem::path GetExecutablePath() { return std::filesystem::path(); }

FRawIcon GetIcon(const std::filesystem::path& path) {
    FRawIcon result;

    return result;
}

bool IsHiddenOrSystem(const std::filesystem::path& p) {
    std::string filename = p.filename().string();

    if (!filename.empty() && filename[0] == '.') {
        return true;
    }

    const std::vector<std::string> linuxSystemFolders = { "lost+found", ".Trash-",
                                                          "System Volume Information" };

    for (const auto& folder : linuxSystemFolders) {
        if (filename.find(folder) != std::string::npos) {
            return true;
        }
    }

    return false;
}

} // namespace ChozoUtils::File