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

void Plang::Parser::NextStatement()
{
	parent->children.push_back({ { InstructionType::Statement }, parent });
	parent = &parent->children.back();
}

Parser::Parser(const Lexer& Lex)
{
	CreateOperators();

	parent = &syntaxTree.root;
	parent->children.push_back({ { InstructionType::Statement }, parent });
	parent = &parent->children.back();

	for (auto i = Lex.tokens.cbegin(); i != Lex.tokens.end(); i++)
		ParseToken(i, Lex.tokens);

	Reparent(&syntaxTree.root, nullptr);
	//second pass to evaluate callables and create named tuples, and further eliminate any single value tuples or empty
	//control structures
	ParseOps(&syntaxTree.root);
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
		parent = parent->parent;

		//handle unbounded tuples
		if (parent->instruction.type == InstructionType::Tuple)
			parent = parent->parent;

		//handle calls
		if (parent->instruction.type == InstructionType::Call)
			parent = parent->parent;

		NextStatement();
	}
	else if (Token->type == LexerTokenType::Separator)
	{
		//because tuples can contain statements, must test two levels
		if ((parent->instruction.type == InstructionType::Tuple
			|| parent->instruction.type == InstructionType::List
			|| parent->instruction.type == InstructionType::Array)
			|| (parent->parent != nullptr
			&& (parent->parent->instruction.type == InstructionType::Tuple
			|| parent->parent->instruction.type == InstructionType::List
			|| parent->parent->instruction.type == InstructionType::Array)))
		{
			parent = parent->parent;
			NextStatement();
		}
		else
		{
			parent = parent->parent;
			auto previous = parent->children.back();
			parent->children.pop_back();

			parent->children.push_back({ { InstructionType::Tuple }, parent });
			parent = &parent->children.back();
			parent->children.push_back(previous);
			Reparent(parent, parent->parent);
			NextStatement(); //todo: only handle if necessary
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
			it = InstructionType::Tuple;
		else if (Token->value == "[")
			it = InstructionType::List;
		else if (Token->value == "[|")
			it = InstructionType::Array;
		else if (Token->value == "{")
		{
			it = InstructionType::Block;

			//test if expression
			auto children = parent->children;
			if (children.size() > 0 && children.back().instruction.type == InstructionType::Tuple)
			{
				//get the arguments
				auto tuple = children.back();
				children.pop_back();

				//create expression
				children.push_back({ { InstructionType::Expression }, parent });
				parent = &children.back();

				//add arguments and block
				parent->children.push_back(tuple);
				parent->children.push_back({ it, parent });

				//make sure tree is up to date and set block to parent
				Reparent(parent, parent->parent);
				parent = &parent->children.back();

				//add statement tuple to block
				NextStatement();

				return;
			}
		}
		parent->children.emplace_back(it, parent);
		parent = &parent->children.back();
		NextStatement();
	}
	else if (Token->type == LexerTokenType::RegionClose)
	{
		//can only close inner-most region
		if (parent->instruction.type == InstructionType::Tuple)
			assert(Token->value == ")");
		else if (parent->instruction.type == InstructionType::List)
			assert(Token->value == "]");
		else if (parent->instruction.type == InstructionType::Array)
			assert(Token->value == "|]");
		else if (parent->parent->instruction.type == InstructionType::Block)
		{
			assert(Token->value == "}");
			parent = parent->parent->parent;
			if (parent->instruction.type == InstructionType::Expression)
				parent = parent->parent; //statement>block>expression
			
			//blocks don't need semicolons to move to the next statement, but should only move if they are in a statement (handling for nesting, such as (a, b, {c:d}, e))
			if (parent->instruction.type == InstructionType::Statement)
			{
				parent = parent->parent; //statement>block>>statement
				NextStatement();
			}

			blocks.pop();
			return;
		}

		if (parent->instruction.type == InstructionType::Statement)
			parent = parent->parent;

		blocks.pop();
		parent = parent->parent;
	}
	else if (Token->type == LexerTokenType::Identifier)
	{
		//handle prefix/postfix
		//-a - b <--- if - and op #2 is also op, use prefix

		/*if (Token->value == "=" || Token->value == ":")
		{
			parent = parent->parent;
			auto previous = parent->children.back();
			parent->children.pop_back();

			parent->children.push_back({ { InstructionType::Call, Token->value }, parent });
			parent = &parent->children.back();
			parent->children.push_back(previous);
			Reparent(parent, parent->parent);
			NextStatement();
		}
		else*/
			parent->children.push_back({ { InstructionType::Identifier, Token->value }, parent });
	}
}
void Parser::ParseNextToken(Lexer::TokenList::const_iterator& Token, const Lexer::TokenList& List)
{
	parent = &parent->children.back();
	auto call = parent;
	Token++;
	if (Token != List.end())
	{
		ParseToken(Token, List);
		//Read nested collections
		while (Token != List.end() && parent != call)
			ParseToken(++Token, List);
		parent = parent->parent;
	}
}

void Parser::ParseOps(SyntaxTreeNode* Statement)
{
	std::vector<SyntaxTreeNode> values, ops;
	for (auto& i : Statement->children)
	{
		if (i.children.size() > 0)
			ParseOps(&i);
		
		if (!i.instruction.value.is<std::string>())
		{
			values.push_back(i);
			continue;
		}

		auto& op = operators.find(i.instruction.value.get<std::string>());
		if (i.instruction.type == InstructionType::Identifier && op != operators.end())
		{
			//should be while new < top, move top to values
			while (ops.size() > 0 && op->second.precedence >= operators.find(ops.back().instruction.value.get<std::string>())->second.precedence)
			{
				values.push_back(ops.back());
				ops.pop_back();
			}
			ops.push_back(i);
		}
		else
			values.push_back(i);
	}
	while (ops.size() > 0)
	{
		values.push_back(ops.back());
		ops.pop_back();
	}

	for (auto& v : values)
		std::cout << ">> " << v.instruction;
	std::cout << std::endl;
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

void Parser::CreateOperator(const std::string& Name, Notation Notat, Association Assoc, unsigned Precedence)
{
	operators[Name] = { Name, Notat, Assoc, Precedence };
}
void Parser::CreateOperators()
{
	CreateOperator("!", Notation::Prefix, Association::None, 3);
	CreateOperator("?", Notation::Postfix, Association::None, 7);
	CreateOperator(",", Notation::Infix, Association::LeftToRight, 8);
	CreateOperator("+", Notation::Infix, Association::RightToLeft, 5);
	CreateOperator("*", Notation::Infix, Association::RightToLeft, 4);
	CreateOperator("=", Notation::Infix, Association::RightToLeft, 10);
	CreateOperator(":", Notation::Infix, Association::RightToLeft, 9);
}
