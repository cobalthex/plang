#include "Pch.hpp"
#include "StreamOps.hpp"

bool StreamOps::IsSequenceNext(std::istream& Stream, const std::string& Sequence)
{
	for (size_t i = 0; !Stream.eof() && i < Sequence.length(); i++)
	{
		if (Stream.peek() != Sequence[i])
		{
			Stream.seekg(-(ptrdiff_t)i, std::ios::cur);
			return false;
		}
		Stream.get();
	}
	return true;
}

std::string StreamOps::ReadUntil(std::istream& Stream, const std::string& Sequence)
{
	std::string s;
	while (!Stream.eof())
	{
		size_t i;
		for (i = 0; !Stream.eof() && i < Sequence.length(); i++)
		{
			s += Stream.get();
			if (s.back() != Sequence[i])
				break;
		}
		if (i == Sequence.length())
		{
			s.resize(s.length() - i);
			Stream.seekg(-(ptrdiff_t)i, std::ios::cur);
			return s;
		}
	}
	return s;
}
std::string StreamOps::ReadUntil(std::istream& Stream, codepoint Char)
{
	std::string s;
	while (!Stream.eof() && Stream.peek() != Char)
		s += Stream.get();
	return s;
}
std::string StreamOps::ReadWhile(std::istream& Stream, std::function<bool(codepoint Char)> ConditionFn)
{
	std::string s;
	while (!Stream.eof() && ConditionFn(Stream.peek()))
		s += Stream.get();
	return s;
}
std::string StreamOps::ReadUntilNewline(std::istream& Stream)
{
	std::string s;
	unsigned ch = 0;
	while (!Stream.eof() && (ch = Stream.peek()) != '\n')
	{
		if (ch == '\r')
		{
			ch = Stream.get();
			if (!Stream.eof() && Stream.peek() == '\n')
				return s;
			s += ch;
		}
		else
			s += Stream.get();
	}
	return s;
}

size_t StreamOps::SkipUntil(std::istream& Stream, const std::string& Sequence)
{
	size_t sz = 0;
	while (!Stream.eof())
	{
		size_t i;
		for (i = 0; !Stream.eof() && i < Sequence.length(); i++)
		{
			sz++;
			if (Stream.get() != Sequence[i])
				break;
		}
		if (i == Sequence.length())
		{
			Stream.seekg(-(ptrdiff_t)i, std::ios::cur);
			return sz - i;
		}
	}
	return sz;
}
size_t StreamOps::SkipUntil(std::istream& Stream, codepoint Char)
{
	size_t sz = 0;
	while (!Stream.eof() && Stream.peek() != Char)
	{
		sz++;
		Stream.get();
	}
	return sz;
}
size_t StreamOps::SkipWhile(std::istream& Stream, std::function<bool(codepoint Char)> ConditionFn)
{
	size_t sz = 0;
	while (!Stream.eof() && ConditionFn(Stream.peek()))
	{
		sz++;
		Stream.get();
	}
	return sz;
}