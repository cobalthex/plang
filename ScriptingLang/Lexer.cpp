#include "pch.hpp"
#include "Lexer.hpp"
#include "StringOps.hpp"

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
	//Stream << " @ " << Token.location;
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
	return !(CharIsWhitespace(Char)
		|| CharIsRegionOpener(Char)
		|| CharIsRegionCloser(Char)
		|| CharIsSpecial(Char)
		|| Char == ';'
		|| Char == ','
		|| Char == '#'
		|| Char == '.');
}

Lexer::Lexer(const std::string& ModuleName, std::istream& Stream)
	: tokens(), lastLine(0), lineNum(1)
{
	while (Stream.good())
	{
		SkipWhitespace(Stream, CharIsWhitespace);
		if (Stream.eof())
			return;

		codepoint ch = 0, nc = 0;

		LexerToken token;
		token.value = ch = Stream.get();
		token.location = { ModuleName, lineNum, (size_t)Stream.tellg() - lastLine };

		if (ch == '#')
		{
			token.type = LexerTokenType::PreprocessCmd;
			Stream >> token.value;
			tokens.push_back(token);
			std::string line;
			std::getline(Stream, line);
			lineNum++;
			lastLine = (size_t)Stream.tellg();

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
			token.type = LexerTokenType::Identifier;
		else if (ch == '.')
		{
			LexerTokenType type;
			if (tokens.size() > 0 && (type = tokens.back().type) != LexerTokenType::Identifier && type != LexerTokenType::Accessor && CharIsNumber(Stream.peek()))
			{
				token.type = LexerTokenType::Number;
				token.value += ReadWhile(Stream, CharIsLiteral);
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
				token.value += ReadUntilNewline(Stream);
			}
			else if (ch == '/' && nc == '*')
			{
				token.type = LexerTokenType::Comment;
				token.value += ReadUntil(Stream, "*/");
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
				token.value += ReadWhile(Stream, CharIsSpecial);
			}
		}
		else if (ch == '\'' || ch == '"')
		{
			token.type = LexerTokenType::String;
			codepoint prev = 0;
			token.value += ReadWhile(Stream, [&](codepoint Char)
			{
				bool cont = (Char != ch || prev == '\\');
				prev = Char;
				return cont;
			});
			token.value += Stream.get();
		}
		else if (CharIsNumber(ch) || ch == '.')
		{
			token.type = LexerTokenType::Number;
			bool dotted = false;
			token.value += ReadWhile(Stream, [&](codepoint Char)
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
			token.value += ReadWhile(Stream, CharIsLiteral);
		}

		tokens.push_back(token);
	}
}

size_t Lexer::SkipWhitespace(std::istream& Stream, const std::function<bool(codepoint Char)>& WhitespaceCmpFn)
{
	size_t sz = 0;
	while (Stream.good() && WhitespaceCmpFn(Stream.peek()))
	{
		sz++;
		if (Stream.get() == '\n')
		{
			lineNum++;
			lastLine = (size_t)Stream.tellg();
		}
	}
	return sz;
}

std::string Lexer::ReadUntil(std::istream& Stream, const std::string& Sequence)
{
	std::string s;
	while (Stream.good())
	{
		size_t i;
		for (i = 0; Stream.good() && i < Sequence.length(); i++)
		{
			auto ch = Stream.get();
			if (ch == '\n')
			{
				lineNum++;
				lastLine = (size_t)Stream.tellg();
			}
			s += ch;
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

std::string Lexer::ReadUntilNewline(std::istream& Stream)
{
	std::string s;
	unsigned ch = 0;
	while (Stream.good() && (ch = Stream.peek()) != '\n')
	{
		if (ch == '\r')
		{
			ch = Stream.get();
			if (Stream.good() && Stream.peek() == '\n')
			{
				lineNum++;
				lastLine = (size_t)Stream.tellg();
				return s;
			}
			s += ch;
		}
		else
			s += Stream.get();
	}

	return s;
}

std::string Lexer::ReadWhile(std::istream& Stream, std::function<bool(codepoint Char)> ConditionFn)
{
	std::string s;
	while (Stream.good() && ConditionFn(Stream.peek()))
	{
		auto ch = Stream.get();
		if (ch == '\n')
		{
			lineNum++;
			lastLine = (size_t)Stream.tellg();
			auto x = 0;
		}
		s += ch;
	}
	return s;
}
