#include "pch.hpp"
#include "Parser.hpp"
#include "StringOps.hpp"

using namespace Plang;

void Parser::Reparent(SyntaxTreeNode* Node, SyntaxTreeNode* Parent)
{
	Node->parent = Parent;
	for (auto& i : Node->children)
		Reparent(&i, Node);
}

std::string Parser::GetRegionSymbol(InstructionType Type)
{
	if (Type == InstructionType::Tuple) return "(";
	if (Type == InstructionType::List) return "[";
	if (Type == InstructionType::Array) return "[|";

	return "";
}

std::string Parser::MatchingRegionSymbol(const std::string& Symbol)
{
	if (Symbol == "{") return "}";
	if (Symbol == "(") return ")";
	if (Symbol == "[") return "]";
	if (Symbol == "[|") return "|]";

	if (Symbol == "}") return "{";
	if (Symbol == ")") return "(";
	if (Symbol == "]") return "[";
	if (Symbol == "|]") return "[|";

	return "";
}

Parser::Parser(const Lexer& Lex)
{
	CreatePredefinedExpressions();

	parent = &syntaxTree.root;

	for (auto i = Lex.tokens.cbegin(); i != Lex.tokens.end(); i++)
		ParseToken(i, Lex.tokens);
	Reparent(parent, nullptr);
	//second pass to evaluate callables (tuples are deconstructed), and further eliminate any single value tuples
	//control structures
}

void Parser::ParseToken(Lexer::TokenList::const_iterator& Token, const Lexer::TokenList& List)
{
	//all arguments are directly following
	if (Token->type == LexerTokenType::PreprocessCmd)
	{
		if (Token->value == "exit")
			Token = List.end() - 1;
	}
	else if (Token->type == LexerTokenType::Number)
	{
		auto number = ParseNumber(Token->value);
		if (number.type == InstructionType::Integer)
			parent->children.push_back({ { number.type, number.value.i }, parent });
		else if (number.type == InstructionType::Float)
			parent->children.push_back({ { number.type, number.value.f }, parent });
	}
	else if (Token->type == LexerTokenType::String)
		parent->children.push_back({ { InstructionType::String, ParseString(Token->value) }, parent });
	else if (Token->type == LexerTokenType::Terminator)
	{
		std::string symbol = GetRegionSymbol(parent->instruction.type);
		if (symbol != "")
		{
			//todo: nested collections
			if (blocks.size() < 1 || blocks.top() != MatchingRegionSymbol(symbol)) //handle unbounded types
				parent = parent->parent;
		}
		else if (parent->instruction.type == InstructionType::Identifier || parent->instruction.type == InstructionType::Call)
		{
			parent = parent->parent;
		}
	}
	else if (Token->type == LexerTokenType::Separator)
	{
		if (parent->instruction.type != InstructionType::Tuple
			&& parent->instruction.type != InstructionType::List
			&& parent->instruction.type != InstructionType::Array)
		{
			//save previous
			auto previous = parent->children.back();
			parent->children.pop_back();

			parent->children.push_back({ { InstructionType::Tuple }, parent });
			parent = &parent->children.back();

			//move previous item to tuple
			parent->children.push_back(previous);
			Reparent(&parent->children.back(), parent);

			Token++;
			size_t n = blocks.size();
			while (Token != List.end())
			{
				ParseToken(Token, List);
				Token++;

				if (Token->type != LexerTokenType::Separator)
					break;
			}
		}
	}
	//chains all acccessors: a.b.c.d => accessor { a, b, c, d }
	else if (Token->type == LexerTokenType::Accessor)
	{
		if (parent->instruction.type != InstructionType::Accessor)
		{
			if (parent->children.size() > 0)
			{
				auto previous = parent->children.back();
				parent->children.pop_back();

				parent->children.push_back({ { InstructionType::Accessor }, parent });
				parent = &parent->children.back();

				parent->children.push_back(previous);
				Reparent(&parent->children.back(), parent);
			}
			else
			{
				parent->children.push_back({ { InstructionType::Accessor }, parent });
				parent = &parent->children.back();
			}
		}
		Token++;
		//read all following accessors
		while (Token != List.end())
		{
			if (Token->type == LexerTokenType::Accessor)
			{
				parent->children.push_back({ { InstructionType::Identifier, "" }, parent }); //multidot properties, .., ...
				Token++;
			}
			else
			{
				ParseToken(Token, List);
				if (Token->type != LexerTokenType::Accessor)
					break;
				Token++;
			}
		}
		parent = parent->parent;
	}
	else if (Token->type == LexerTokenType::RegionOpen)
	{
		blocks.push(Token->value);

		InstructionType it;
		if (Token->value == "(")
		{
			it = InstructionType::Tuple;
		}
		else if (Token->value == "[")
			it = InstructionType::List;
		else if (Token->value == "[|")
			it = InstructionType::Array;
		else if (Token->value == "{")
		{
			it = InstructionType::Block;

			//test if expression
			auto& children = parent->children;
			if (children.size() > 0 && (children.back().instruction.type == InstructionType::Tuple || children.back().instruction.type == InstructionType::NamedTuple))
			{
				auto tuple = children.back();
				children.pop_back();
				children.push_back({ { InstructionType::Expression }, parent });
				parent = &children.back();

				parent->children.push_back(tuple);
				parent->children.push_back({ it, parent });

				Reparent(parent, parent->parent);
				parent = &parent->children.back();

				return;
			}
		}
		parent->children.emplace_back(it, parent);
		parent = &parent->children.back();
	}
	else if (Token->type == LexerTokenType::RegionClose)
	{
		//can only close inner-most region
		if (parent->instruction.type == InstructionType::Tuple || parent->instruction.type == InstructionType::NamedTuple)
			assert(Token->value == ")");
		if (parent->instruction.type == InstructionType::List)
			assert(Token->value == "]");
		if (parent->instruction.type == InstructionType::Array)
			assert(Token->value == "|]");
		if (parent->instruction.type == InstructionType::Block)
		{
			assert(Token->value == "}");
			if (parent->parent->instruction.type == InstructionType::Expression)
				parent = parent->parent;
		}

		blocks.pop();
		parent = parent->parent;
	}
	else if (Token->type == LexerTokenType::Identifier)
	{
		auto pExpr = predefinedExpressions.find(Token->value);
		if (pExpr != predefinedExpressions.end())
		{
			auto& expr = pExpr->second;
			if (expr.notation == Notation::Prefix)
			{
				parent->children.push_back({ { InstructionType::Call, Token->value }, parent });
				parent = &parent->children.back();
				Token++;
				ParseToken(Token, List);
				parent = parent->parent;
			}
			else
				parent->children.push_back({ { InstructionType::Identifier, Token->value }, parent });
		}
		else
			parent->children.push_back({ { InstructionType::Identifier, Token->value }, parent });
	}
}


Number Parser::ParseNumber(std::string Input)
{
	StringOps::Replace(Input, "'", "");
	if (Input.find('.') != Input.npos)
	{
		return { InstructionType::Float, std::stod(Input) };
	}
	else
	{
		int base = 10;
		return { InstructionType::Integer, std::stoll(Input, nullptr, base) };
	}
}
std::string Parser::ParseString(std::string Input)
{
	bool interpolate = Input[0] == '"';
	Input = Input.substr(1, Input.length() - 2);
	return Input;
}

void Parser::CreateExpression(const std::string& Name, Notation Notat, Association Assoc, unsigned Precedence)
{
	predefinedExpressions[Name] = { Name, Notat, Assoc, Precedence };
}
void Parser::CreatePredefinedExpressions()
{
	CreateExpression("!", Notation::Prefix, Association::LeftToRight, 3);
	CreateExpression("+", Notation::Infix, Association::RightToLeft, 5);
	CreateExpression("*", Notation::Infix, Association::RightToLeft, 5);
	CreateExpression("=", Notation::Infix, Association::RightToLeft, 10);
	CreateExpression(":", Notation::Infix, Association::RightToLeft, 10);
}
