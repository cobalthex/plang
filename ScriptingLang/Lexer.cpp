#include "pch.hpp"
#include "Lexer.hpp"

using namespace Plang;

std::ostream& operator << (std::ostream& Stream, const Plang::LexerToken& Token)
{
	static const std::string types[] = {
		"invalid",
		"Comment",
		"Terminator",
        "TupleOpen",
        "TupleClose",
		"ListOpen",
		"ListClose",
        "BlockOpen",
        "BlockClose",
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
		Stream << i << "\n";
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
    switch (Char)
    {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case ';':
    case ',':
    case '.':
        return false;
    default:
        return !CharIsWhitespace(Char) &&
               !CharIsSpecial(Char);
    }
}

Lexer::Lexer(const std::string& ModuleName, std::istream& Stream)
	: tokens(), lastLine(0), lineNum(1)
{
	if (!Stream.good())
		return;

	while (true)
	{
		SkipWhitespace(Stream, CharIsWhitespace);

		codepoint ch = 0, nc = 0;

		LexerToken token;
		token.value = ch = Stream.get();
		token.location = { ModuleName, lineNum, (size_t)Stream.tellg() - lastLine };

		if (ch < 0)
			return;
		else if (ch == ';')
			token.type = LexerTokenType::Terminator;
		else if (ch == ',')
			token.type = LexerTokenType::Separator;
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
        else if (ch == '(')
            token.type = LexerTokenType::TupleOpen;
        else if (ch == ')')
            token.type = LexerTokenType::TupleClose;
		else if (ch == '[')
			token.type = LexerTokenType::ListOpen;
		else if (ch == ']')
			token.type = LexerTokenType::ListClose;
        else if (ch == '{')
            token.type = LexerTokenType::BlockOpen;
        else if (ch == '}')
            token.type = LexerTokenType::BlockClose;
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
	codepoint ch = 0;
	while ((ch = Stream.peek()) >= 0 && WhitespaceCmpFn(ch))
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
	while (true)
	{
		size_t i;
		for (i = 0; i < Sequence.length(); i++)
		{
			codepoint ch = Stream.get();
			if (ch < 0)
				return s;

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
	codepoint ch = 0;
	while ((ch = Stream.peek()) >= 0 && ch != '\n')
	{
		if (ch == '\r')
		{
			ch = Stream.get();
			if (ch < 0)
				return s;

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
	codepoint ch = 0;
	while ((ch = Stream.peek()) >= 0 && ConditionFn(ch))
	{
		ch = Stream.get();
		if (ch == '\n')
		{
			lineNum++;
			lastLine = (size_t)Stream.tellg();
		}
		s += ch;
	}
	return s;
}
