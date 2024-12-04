#include "Chozo/Utilities/PlatformUtils.h"

std::string OpenFileDialog();
std::string SaveFileDialog(const std::string& defaultFileName);

namespace Chozo {

    std::string FileDialogs::OpenFile(const char* filter)
    {
        return OpenFileDialog();
    }

    std::string FileDialogs::SaveFile(const char* filter, const std::string& defaultFileName)
    {
        return SaveFileDialog(defaultFileName);
    }
}