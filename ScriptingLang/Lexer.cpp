#include "pch.hpp"
#include "Lexer.hpp"
#include "StringOps.hpp"
#include "StreamOps.hpp"

using namespace Plang;

std::ostream& operator << (std::ostream& Stream, const Plang::LexerToken& Token)
{
	static const std::string types[] = {
		"invalid",
		"Comment",
		"PreprocessCmd",
		"PreprocessArg",
		"Terminator",
		"RegionOpen",
		"RegionClose",
		"Separator",
		"Identifier",
		"Accessor",
		"Number",
		"String",
	};
	Stream << std::right << std::setw(13) << types[(size_t)Token.type] << " " << Token.value;
	return Stream;
}
std::ostream& operator << (std::ostream& Stream, const Plang::Lexer& Lexer)
{
	for (auto& i : Lexer.tokens)
		Stream << i << std::endl;
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
	return !(CharIsWhitespace(Char) || CharIsRegionOpener(Char) || CharIsRegionCloser(Char) || CharIsSpecial(Char) || Char == ';' || Char == ',' || Char == '#' || Char == '.');
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
			token.type = LexerTokenType::PreprocessCmd;
			Stream >> token.value;
			tokens.push_back(token);
			std::string line;
			std::getline(Stream, line);

			token.type = LexerTokenType::PreprocessArg;
			size_t next = 0;
			for (size_t i = 0; i < line.length(); i++)
			{
				if (CharIsWhitespace(line[i]))
				{
					if (i - next > 0)
					{
						token.value = line.substr(next, i - next);
						tokens.push_back(token);
					}
					next = i + 1;
				}
			}
			token.value = line.substr(next);
			tokens.push_back(token);
			continue;
		}
		else if (ch == ';')
			token.type = LexerTokenType::Terminator;
		else if (ch == ',')
			token.type = LexerTokenType::Separator;
		else if (ch == '.')
		{
			ch = Stream.unget().unget().get();
			Stream.get();

			//read numbers like .5
			if (tokens.size() < 1 || CharIsWhitespace(ch) || CharIsSpecial(ch) || ch == ';' || ch == '#')
			{
				token.type = LexerTokenType::Number;
				token.value += StreamOps::ReadWhile(Stream, CharIsLiteral);
			}
			else
				token.type = LexerTokenType::Accessor;
		}
		else if (CharIsRegionOpener(ch))
		{
			token.type = LexerTokenType::RegionOpen;
			if (Stream.peek() == '|') //arrays
				token.value += Stream.get();
		}
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
			codepoint prev = 0;
			token.value += StreamOps::ReadWhile(Stream, [&](codepoint Char) { bool cont = (Char != ch || prev == '\\'); prev = Char; return cont; });
			token.value += Stream.get();
		}
		else if (CharIsNumber(ch) || ch == '.')
		{
			token.type = LexerTokenType::Number;
			bool dotted = false;
			token.value += StreamOps::ReadWhile(Stream, [&dotted](codepoint Char)
			{
				bool cont = CharIsLiteral(Char);
				if (Char == '.')
				{
					cont = !dotted;
					dotted = true;
				}
				return cont;
			});
		}
		else
		{
			token.type = LexerTokenType::Identifier;
			token.value += StreamOps::ReadWhile(Stream, [](codepoint Char) { return CharIsLiteral(Char); });
		}

		tokens.push_back(token);
	}
}
