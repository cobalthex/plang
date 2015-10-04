#include "pch.hpp"
#include "Parser.hpp"

using namespace Plang;

void Parser::CreatePredefinedIdentifiers()
{
	ParseIdentifier pi;

	pi.name = "+";
	pi.association = Association::LeftToRight;
	pi.precedence = 5;
	predefinedIdentifiers[pi.name] = pi;

	pi.name = "*";
	pi.association = Association::LeftToRight;
	pi.precedence = 4;
	predefinedIdentifiers[pi.name] = pi;
}

Parser::Parser(const Lexer& Lex)
{
	CreatePredefinedIdentifiers();
	
	for (auto& tok : Lex.tokens)
	{

	}
}