#include "pch.hpp"
#include "Lexer.hpp"
#include "StringOps.hpp"
#include "StreamOps.hpp"

using namespace Plang;

std::ostream& operator << (std::ostream& Stream, LexerTokenType TokenType)
{
	std::string types[] = {
		"invalid",
		"Comment",
		"Preprocessor",
		"Terminator",
		"RegionOpen",
		"RegionClose",
		"Separator",
		"Identifier",
		"Number",
		"String",
	};
	Stream << types[(size_t)TokenType];
	return Stream;
}

bool Lexer::CharIsWhitespace(codepoint Char)
{
	if (Char <= ' ')
		return true;
	switch (Char)
	{
	case 160:
		return true;

	default:
		return false;
	}
}
bool Lexer::CharIsRegionOpener(codepoint Char)
{
	switch (Char)
	{
	case '(':
	case '[':
	case '{':
		return true;

	default:
		return false;
	}
}
bool Lexer::CharIsRegionCloser(codepoint Char)
{
	switch (Char)
	{
	case ')':
	case ']':
	case '}':
		return true;

	default:
		return false;
	}
}
bool Lexer::CharIsSpecial(codepoint Char)
{
	switch (Char)
	{
	case '.':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '!':
	case '^':
	case '&':
	case '|':
	case '~':
	case '=':
	case ':':
	case '?':
	case '<':
	case '>':
		return true;
	default:
		return false;
	}
}

bool Lexer::CharIsLiteral(codepoint Char)
{
	return !(CharIsWhitespace(Char) || CharIsRegionOpener(Char) || CharIsRegionCloser(Char) || CharIsSpecial(Char) || Char == ';' || Char == ',' || Char == '#');
}

Lexer::Lexer(std::istream& Stream)
{
	while (!Stream.eof())
	{
		StreamOps::SkipWhitespace(Stream, CharIsWhitespace);
		if (Stream.eof())
			return;

		LexerToken token;
		codepoint ch = 0, nc = 0;
		token.value = ch = Stream.get();
		
		if (ch == '#')
		{
			token.type = LexerTokenType::Preprocessor;
			token.value = StreamOps::ReadUntilNewline(Stream);
		}
		else if (ch == ';')
			token.type = LexerTokenType::Terminator;
		else if (ch == ',')
			token.type = LexerTokenType::Separator;
		else if (CharIsRegionOpener(ch))
			token.type = LexerTokenType::RegionOpen;
		else if (CharIsRegionCloser(ch))
			token.type = LexerTokenType::RegionClose;
		else if (CharIsSpecial(ch))
		{
			nc = Stream.peek();
			if (ch == '/' && nc == '/')
			{
				token.type = LexerTokenType::Comment;
				token.value += StreamOps::ReadUntilNewline(Stream);
			}
			else if (ch == '/' && nc == '*')
			{
				token.type = LexerTokenType::Comment;
				token.value += StreamOps::ReadUntil(Stream, "*/");
				token.value += "*/";
				Stream.seekg(2, std::ios::cur);
			}
			else if (ch == '|' && nc == ']')
			{
				token.type = LexerTokenType::RegionClose;
				token.value += Stream.get();
			}
			else
			{
				token.type = LexerTokenType::Identifier;
				token.value += StreamOps::ReadWhile(Stream, [](codepoint Char) { return CharIsSpecial(Char); });
			}
		}
		else if (ch == '\'' || ch == '"')
		{
			token.type = LexerTokenType::String;
			token.value += StreamOps::ReadUntil(Stream, ch);
			token.value += Stream.get();
		}
		else if (CharIsNumber(ch) || ch == '.')
		{
			token.type = LexerTokenType::Number;
			token.value += StreamOps::ReadWhile(Stream, [](codepoint Char) { return CharIsLiteral(Char); });
		}
		else
		{
			token.type = LexerTokenType::Identifier;
			token.value += StreamOps::ReadWhile(Stream, [](codepoint Char) { return CharIsLiteral(Char); });
		}

		tokens.push_back(token);
	}
}