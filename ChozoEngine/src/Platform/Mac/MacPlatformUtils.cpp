#include "Chozo/Utils/PlatformUtils.h"

std::string OpenFileDialog();
std::string SaveFileDialog();

namespace Chozo {

    std::string FileDialogs::OpenFile(const char* filter)
    {
        return OpenFileDialog();
    }

    std::string FileDialogs::SaveFile(const char* filter)
    {
        return SaveFileDialog();
    }
}