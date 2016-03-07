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

bool Parser::IsRegion(const Instruction& Instruction)
{
	return (Instruction.type == InstructionType::Tuple
		|| Instruction.type == InstructionType::List
		|| Instruction.type == InstructionType::Array
		|| Instruction.type == InstructionType::Block);
}

Parser::Parser(const Lexer& Lex)
{
	CreateOperators();

	parent = &syntaxTree.root;
	parent->children.push_back({ { InstructionType::Statement }, parent, Location() });
	parent = &parent->children.back();

	for (auto i = Lex.tokens.cbegin(); i != Lex.tokens.end(); i++)
		ParseToken(i, Lex.tokens);

	Reparent(&syntaxTree.root, nullptr);
	//second pass to evaluate callables and create named tuples, and further eliminate any single value tuples or empty
	//control structures
	//ParseOps(&syntaxTree.root);
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
			parent->children.push_back({ { number.type, number.value.i }, parent, Token->location });
		else if (number.type == InstructionType::Float)
			parent->children.push_back({ { number.type, number.value.f }, parent, Token->location });
	}
	else if (Token->type == LexerTokenType::String)
		parent->children.push_back({ { InstructionType::String, ParseString(Token->value) }, parent, Token->location });
	else if (Token->type == LexerTokenType::Terminator)
	{
		//todo possibly handle (1;2;3)

		while (parent->instruction.type != InstructionType::Statement)
			parent = parent->parent;

		//collapse empty statements
		if (parent->children.size() < 1)
			return;

		parent = parent->parent;

		if (parent->instruction.type != InstructionType::Program && parent->instruction.type != InstructionType::Block)
			throw "Invalid terminator";

		parent->children.push_back({ { InstructionType::Statement }, parent, Token->location });
		parent = &parent->children.back();
	}
	else if (Token->type == LexerTokenType::Separator)
	{
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

				parent->children.push_back({ { InstructionType::Accessor }, parent, Token->location });
				parent = &parent->children.back();

				parent->children.push_back(previous);
				Reparent(&parent->children.back(), parent);
			}
			else
			{
				parent->children.push_back({ { InstructionType::Accessor }, parent, Token->location });
				parent = &parent->children.back();
			}
		}
		Token++;
		//read all following accessors
		while (Token != List.end())
		{
			if (Token->type == LexerTokenType::Accessor)
			{
				parent->children.push_back({ { InstructionType::Identifier, "" }, parent, Token->location }); //multidot properties, .., ...
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
		if (Token->value == "{")
		{
			//if previous particle is tuple, convert to expression
			if (parent->children.size() > 0 && parent->children.back().instruction.type == InstructionType::Tuple)
			{
				auto tuple = parent->children.back();
				parent->children.pop_back();

				parent->children.push_back({ { InstructionType::Expression }, parent, tuple.location });
				auto& node = parent->children.back();

				node.children.push_back(tuple);
				tuple.parent = &node;

				node.children.push_back({ { InstructionType::Block }, &node, Token->location });
				parent = &node.children.back();
			}
			else
			{
				parent->children.push_back({ { InstructionType::Block }, parent, Token->location });
				parent = &parent->children.back();
			}
		}
		else
		{
			parent->children.push_back({ { InstructionType::Unknown }, parent, Token->location });
			parent = &parent->children.back();

			if (Token->value == "(")
				parent->instruction.type = InstructionType::Tuple;
			else if (Token->value == "[")
				parent->instruction.type = InstructionType::List;
			else if (Token->value == "[|")
				parent->instruction.type = InstructionType::Array;
		}
	}
	else if (Token->type == LexerTokenType::RegionClose)
	{
		while (!IsRegion(parent->instruction))
		{
			if (parent->parent == nullptr || parent->parent->instruction.type == InstructionType::Program)
				throw ("Unmatched region close @ " + (std::string)Token->location);
			parent = parent->parent;
		}

		//single statement tuples become parens (todo: handle expressions, (a) { } converts to a { })
		/*if (parent->instruction.type == InstructionType::Tuple && parent->children.size() == 1)
		{
			auto node = parent->children.front();
			node.parent = parent->parent;
			parent->parent->children.pop_back();
			parent->parent->children.push_back(node);
		}*/

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
			parent->children.push_back({ { InstructionType::Identifier, Token->value }, parent, Token->location });
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
	size_t opc = 0;
	std::vector<SyntaxTreeNode> values, ops;
	for (auto& i : Statement->children)
	{
		if (i.children.size() > 0)
			ParseOps(&i);
		
		if (!i.instruction.value.is<std::string>())
		{
			values.push_back(i);
			opc++;
			continue;
		}

		auto op = operators.find(i.instruction.value.get<std::string>());
		if (i.instruction.type == InstructionType::Identifier && op != operators.end())
		{
			//should be while new < top, move top to values
			while (ops.size() > 0 && op->second.precedence >= operators.find(ops.back().instruction.value.get<std::string>())->second.precedence)
			{
				values.push_back(ops.back());
				ops.pop_back();
			}
			ops.push_back(i);
			opc = 0;
		}
		else
		{
			values.push_back(i);
			opc++;
		}

		/*if (opc >= 2)
			throw "invalid syntax, cannot be unescaped";*/
	}
	while (ops.size() > 0)
	{
		values.push_back(ops.back());
		ops.pop_back();
	}

	if (values.size() < 1)
		return;

	Statement->children.clear();
	parent = Statement;

	for (size_t i = values.size() - 1; i > 0; i--)
	{
		auto& value = values[i];

		parent->children.insert(parent->children.begin(), value);

		if (value.instruction.value.is<std::string>())
		{
			auto op = operators.find(value.instruction.value.get<std::string>());
			if (op != operators.end())
			{
				parent = &parent->children.front();
				continue;
			}
		}
		if (parent->instruction.type != InstructionType::Statement && parent->children.size() > 1)
			parent = parent->parent;
	}
	parent = Statement;
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
