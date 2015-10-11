#include "pch.hpp"
#include "StringOps.hpp"

std::string& StringOps::Replace(std::string& String, const std::string& Find, const std::string& Replace)
{
	size_t pos = 0;
	while ((pos = String.find(Find, pos)) != std::string::npos)
	{
		String.replace(pos, Find.length(), Replace);
		pos += Replace.length();
	}
	return String;
}
std::string& StringOps::Replace(std::string& String, const char* Find, const char* Replace)
{
	auto findLen = strlen(Find);
	auto repLen = strlen(Replace);
	size_t pos = 0;
	while ((pos = String.find(Find, pos)) != std::wstring::npos)
	{
		String.replace(pos, findLen, Replace);
		pos += repLen;
	}
	return String;
}