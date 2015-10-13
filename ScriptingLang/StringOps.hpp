#pragma once

#include "pch.hpp"

namespace StringOps
{
	inline bool StartsWith(const std::string& String, const std::string& Prefix)
	{
		if (Prefix.length() > String.length())
			return false;
		return std::equal(Prefix.begin(), Prefix.end(), String.begin());
	}
	inline std::string& ToUpper(std::string& String)
	{
		static std::locale loc = std::locale();

		for (size_t i = 0; i < String.length(); i++)
			String[i] = std::toupper(String[i], loc);
		return String;
	}

	extern std::string& Replace(std::string& String, const std::string& Find, const std::string& Replace); //in place replace
	extern std::string& Replace(std::string& String, const char* Find, const char* Replace);

	inline std::string Replace(const std::string& String, const std::string& Find, const std::string& Replace) { std::string s (String); return StringOps::Replace(s, Find, Replace); }
	inline std::string Replace(const std::string& String, const char* Find, const char* Replace) { std::string s (String); return StringOps::Replace(s, Find, Replace); }
};
