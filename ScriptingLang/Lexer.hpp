#pragma once
#include "pch.hpp"

namespace Plang
{
	enum class LexerTokenType : size_t
	{
		Invalid,
		Comment,
		PreprocessCmd,
		PreprocessArg,
		Terminator,
		RegionOpen,
		RegionClose,
		Separator,
		Identifier,
		Accessor,
		Number,
		String
	};

	struct LexerToken
	{
		LexerTokenType type;
		std::string value;

		LexerToken() = default;
	};

	class Lexer
	{
	public:
		Lexer() = default;
		Lexer(std::istream& Input);
		~Lexer() = default;

		static bool CharIsWhitespace(codepoint Char);
		static bool CharIsRegionOpener(codepoint Char);
		static bool CharIsRegionCloser(codepoint Char);
		static inline bool CharIsNumber(codepoint Char) { return (Char >= '0' && Char <= '9'); }
		static bool CharIsSpecial(codepoint Char); //characters that are valid sub-symbol characters (can only be by themselves, e.g: + - * %)
		static bool CharIsLiteral(codepoint Char); //char is not a splitter, encloser, or whitespace

		std::vector<LexerToken> tokens;
	};
};
extern std::ostream& operator << (std::ostream& Stream, const Plang::Lexer& Lexer);