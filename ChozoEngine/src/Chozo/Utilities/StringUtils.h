#pragma once

namespace Chozo::Utils {

    namespace String {

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
		
    }

	namespace WChar {

		inline uint64_t WCharToUint64(const wchar_t* wchar_str)
		{
			std::wstring wstr(wchar_str);
			std::string str(wstr.begin(), wstr.end());
			uint64_t result = std::stoull(str);

			return result;
		}
	}
}