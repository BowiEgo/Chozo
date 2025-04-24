#pragma once

#include <codecvt>

#if defined(CZ_PLATFORM_WIN)
	#include <windows.h>
#else
#endif

namespace Chozo::Utils {

    namespace String {

		inline bool isNumeric(const std::string& str)
		{
			std::regex pattern(R"(^\d+$)");
			return std::regex_match(str, pattern);
		}

        inline std::string& ToLower(std::string& string)
		{
			std::transform(string.begin(), string.end(), string.begin(),
						   [](const unsigned char c) { return std::tolower(c); });
			return string;
		}

        inline std::string ToLowerCopy(const std::string_view string)
		{
			std::string result(string);
			ToLower(result);
			return result;
		}
		
		inline uint64_t ToUint64(std::string string)
		{
			uint64_t result = isNumeric(string) ? std::stoull(string) : 0;
			return result;
		}
    }

	namespace Numeric {
		inline std::string FormatWithCommas(uint64_t value) {
			std::stringstream ss;
			ss.imbue(std::locale("en_US.UTF-8")); // Locale adds comma grouping
			ss << std::fixed << value;
			return ss.str();
		}
	}

	namespace WChar {

#ifdef CZ_PLATFORM_WIN
    	inline std::string WStringToString(const std::wstring& wstr)
    	{
    		if (wstr.empty()) return {};
    		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    		std::string strTo(size_needed, 0);
    		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    		return strTo;
    	}
#endif

		inline uint64_t WCharToUint64(const wchar_t* wchar_str)
		{
			if (!wchar_str) return 0;
#ifdef CZ_PLATFORM_WIN
			std::wstring wstr(wchar_str);
			std::string str = WStringToString(wstr);
    		return std::stoull(str);
#else
    		return std::wcstoull(wchar_str, nullptr, 10);
#endif
		}
	}
}