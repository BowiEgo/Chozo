#pragma once

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