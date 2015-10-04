#pragma once

#include "pch.hpp"

//Helper functions for streams
namespace StreamOps
{
	//Skip whitespace characters (defined by WhitespaceCmp). Returns the number of whitespace characters counted
	inline size_t SkipWhitespace(std::istream& Stream, const std::function<bool(codepoint Char)>& WhitespaceCmpFn)
	{
		size_t sz = 0;
		while (!Stream.eof() && WhitespaceCmpFn(Stream.peek()))
		{
			sz++;
			Stream.get();
		}
		return sz;
	}

	//Are the next characters in the stream the sequence?
	//If no, position is rewound, otherwise, true is returned and the stream is at oldpos + Sequence.length
	bool IsSequenceNext(std::istream& Stream, const std::string& Sequence);

	//Read the stream until Sequence is found
	std::string ReadUntil(std::istream& Stream, const std::string& Sequence);
	std::string ReadUntil(std::istream& Stream, codepoint Char);

	std::string ReadWhile(std::istream& Stream, std::function<bool(codepoint Char)> ConditionFn); //read while the condition is true

	//read until a newline -- automatically strips \r before a \n
	std::string ReadUntilNewline(std::istream& Stream);

	//ignore characters in the stream until Sequence is found
	size_t SkipUntil(std::istream& Stream, const std::string& Sequence);
	size_t SkipUntil(std::istream& Stream, codepoint Char);

	size_t SkipWhile(std::istream& Stream, std::function<bool(codepoint Char)> ConditionFn); //skip while the condition is true
};