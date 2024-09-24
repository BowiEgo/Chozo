#pragma once

#include <string>

namespace Chozo {

    class FileDialogs
    {
    public:
        // Return empty strings if cancelled
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter, const std::string& defaultFileName = "");
    };
}
