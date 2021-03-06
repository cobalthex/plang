#pragma once
#include "pch.hpp"
#include "Location.hpp"

namespace Plang
{
	enum class LexerTokenType
	{
		Invalid,
		Comment,
		Terminator,
        TupleOpen,
        TupleClose,
		ListOpen,
		ListClose,
        BlockOpen,
        BlockClose,
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
		Location location;

		LexerToken() = default;
        LexerToken(LexerTokenType type, const std::string& value, Location location)
            : type(type), value(value), location(location) { }
	};

	//todo: change to work like stringstream

	class Lexer
	{
	public:
		using TokenList = std::vector<LexerToken>;

		Lexer() = default;
		Lexer(const std::string& ModuleName, std::istream& Input);

		static bool CharIsWhitespace(codepoint Char);
		static inline bool CharIsNumber(codepoint Char) { return (Char >= '0' && Char <= '9'); }
		static bool CharIsSpecial(codepoint Char); //characters that are valid sub-symbol characters (can only be by themselves, e.g: + - * %)
		static bool CharIsLiteral(codepoint Char); //char is not a splitter, encloser, or whitespace

		TokenList tokens;

	protected:
		size_t lastLine; //last lineNum
		size_t lineNum;

		//Skip whitespace characters (defined by WhitespaceCmp). Returns the number of whitespace characters counted
		size_t SkipWhitespace(std::istream& Stream, const std::function<bool(codepoint Char)>& WhitespaceCmpFn);
		std::string ReadUntil(std::istream& Stream, const std::string& Sequence);
		std::string ReadUntilNewline(std::istream& Stream);
		std::string ReadWhile(std::istream& Stream, std::function<bool(codepoint Char)> ConditionFn);

	};
};
extern std::ostream& operator << (std::ostream& Stream, const Plang::Lexer& Lexer);
extern std::ostream& operator << (std::ostream& Stream, const Plang::LexerToken& Token);
