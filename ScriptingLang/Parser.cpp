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
		|| Instruction.type == InstructionType::Block
		|| Instruction.type == InstructionType::Call
		|| Instruction.type == InstructionType::ControlStructure);
}

Parser::Parser(const Lexer::TokenList& Tokens)
{
	CreateOperators();

	parent = &syntaxTree.root;
	parent->children.push_back({ { InstructionType::Statement }, parent, Location() });
	parent = &parent->children.back();

	for (size_t i = 0; i < Tokens.size(); i++)
	{
		auto itr = Tokens.cbegin() + i;
		ParseToken(itr, Tokens);
	}

	ParseStatement(&syntaxTree.root.children.back());

	if (syntaxTree.root.children.back().instruction.type == InstructionType::Statement && syntaxTree.root.children.back().children.size() < 1)
		syntaxTree.root.children.pop_back();

	//second pass to evaluate callables and create named tuples, and further eliminate any single value tuples or empty
	//control structures
	//ParseOps(&syntaxTree.root);
}

Parser& Parser::operator = (const Parser& Other)
{
	if (this != &Other)
	{
		syntaxTree = Other.syntaxTree;
		operators = Other.operators;

		Reparent(&syntaxTree.root, nullptr);
	}

	return *this;
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

		SyntaxTreeNode node (number, parent, Token->location);

		if ((Token != List.cbegin() && (Token - 1)->type == LexerTokenType::Accessor) ||
			(parent->children.size() > 0 && parent->children.back().instruction.type == InstructionType::Accessor))
			parent->children.back().children.push_back(node);
		else
			parent->children.push_back(node);
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

		ParseStatement(parent);
		parent = parent->parent;

		if (parent->instruction.type != InstructionType::Program && parent->instruction.type != InstructionType::Block)
			throw ParserException("Invalid terminator", Token->value, Token->location);

		parent->children.push_back({ { InstructionType::Statement }, parent, Token->location });
		parent = &parent->children.back();
	}
	else if (Token->type == LexerTokenType::Separator)
	{
		while (parent->instruction.type != InstructionType::Statement)
			parent = parent->parent;

		ParseStatement(parent);
		parent = parent->parent;

		parent->children.push_back({ { InstructionType::Statement }, parent, Token->location });
		parent = &parent->children.back();
	}
	//chains all acccessors: a.b.c.d => accessor { a, b, c, d }
	else if (Token->type == LexerTokenType::Accessor)
	{
		if (parent->children.size() > 0 && parent->children.back().instruction.type == InstructionType::Accessor)
			return;
		else if (Token != List.cbegin() && (Token - 1)->type == LexerTokenType::Accessor)
			parent->children.back().children.push_back({ { InstructionType::Identifier, "" }, parent, Token->location });
		else if (parent->instruction.type != InstructionType::Accessor)
		{
			auto particle = parent->children.back();
			parent->children.pop_back();

			parent->children.push_back({ { InstructionType::Accessor }, parent, Token->location });
			auto& node = parent->children.back();

			node.children.push_back(particle);
			particle.parent = &node;
		}
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
			else if (parent->children.size() > 0 && parent->children.back().instruction.type == InstructionType::Call)
			{
				parent = &parent->children.back();
				parent->instruction.type = InstructionType::ControlStructure;

				SyntaxTreeNode tuple { { InstructionType::Tuple }, parent, Token->location };
				tuple.children = std::move(parent->children);
				parent->children.clear();
				parent->children.push_back(tuple);

				parent->children.push_back({ { InstructionType::Block }, parent, Token->location });
				parent = &parent->children.back();
			}
			else
			{
				parent->children.push_back({ { InstructionType::Block }, parent, Token->location });
				parent = &parent->children.back();
			}
		}
		else if (Token->value == "(" && parent->children.size() > 0 && parent->children.back().instruction.type == InstructionType::Identifier)
		{
			auto& ident = parent->children.back();
			ident.instruction.type = InstructionType::Call;
			parent = &parent->children.back();
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

		parent->children.push_back({ { InstructionType::Statement }, parent, Token->location });
		parent = &parent->children.back();
	}
	else if (Token->type == LexerTokenType::RegionClose)
	{
		if (parent->instruction.type == InstructionType::Statement && parent->children.size() < 1)
		{
			parent = parent->parent;
			parent->children.pop_back();
		}
		else
			ParseStatement(parent);

		while (!IsRegion(parent->instruction))
		{
			if (parent->parent == nullptr || parent->parent->instruction.type == InstructionType::Program)
				throw ParserException("Unknown region close", Token->value, Token->location);
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

		if (parent->instruction.type == InstructionType::ControlStructure)
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

		SyntaxTreeNode node = { { InstructionType::Identifier, Token->value }, parent, Token->location };

		if ((Token != List.cbegin() && (Token - 1)->type == LexerTokenType::Accessor) ||
			(parent->children.size() > 0 && parent->children.back().instruction.type == InstructionType::Accessor))
			parent->children.back().children.push_back(node);
		else
			parent->children.push_back(node);
	}
}

void Parser::ParseStatement(SyntaxTreeNode* Statement)
{
	auto _parent = Statement->parent;

	if (Statement->children.size() < 1)
		return;

	if (Statement->children.size() == 1)
	{
		new (Statement) SyntaxTreeNode(Statement->children.back());
		Reparent(Statement, _parent);
		return;
	}

	std::vector<SyntaxTreeNode> output, ops;
	for (auto& i : Statement->children)
	{
		//only identifiers can be operators
		if (i.instruction.type != InstructionType::Identifier)
		{
			output.push_back(i);
			continue;
		}

		auto op = operators.find(i.instruction.value.get<std::string>());
		if (op != operators.end())
		{
			//should be while new < top, move top to values
			while (ops.size() > 0)
			{
				auto precedence = operators.find(ops.back().instruction.value.get<std::string>())->second.precedence;

				if ((op->second.association == Association::LeftToRight && op->second.precedence <= precedence) ||
					(op->second.association == Association::RightToLeft && op->second.precedence < precedence))
					break;

				output.push_back(ops.back());
				ops.pop_back();
			}
			ops.push_back(i);
		}
		else
		{
			output.push_back(i);
		}
	}

	if (ops.size() < 1)
		throw ParserException("Unexpected identifier", output.back().instruction, output.back().location); //where bare words support should go

	while (ops.size() > 1)
	{
		output.push_back(ops.back());
		ops.pop_back();
	}

	new (Statement) SyntaxTreeNode(ops.back());
	Reparent(Statement, _parent);
	Statement->instruction.type = InstructionType::Call;

	_parent = Statement;
	while (output.size() > 0)
	{
		if (output.back().instruction.type == InstructionType::Identifier && operators.find(output.back().instruction.value.get<std::string>()) != operators.end())
		{
			_parent->children.insert(_parent->children.begin(), output.back());
			Reparent(&_parent->children.front(), _parent);
			_parent->children.front().instruction.type = InstructionType::Call;

			_parent = &_parent->children.front();
		}
		else
		{
			_parent->children.insert(_parent->children.begin(), output.back());
			Reparent(&_parent->children.front(), _parent);

			if (_parent->children.size() > 1)
				_parent = _parent->parent;
		}

		output.pop_back();
	}
}

Instruction Parser::ParseNumber(std::string Input)
{
	StringOps::Replace(Input, "'", "");

	int base = 10;
	auto baseOffset = Input.find_last_of('_');
	if (baseOffset != std::string::npos)
	{
		auto baseStr = Input.substr(baseOffset + 1);
		Input.resize(baseOffset);
		base = std::stoi(baseStr);
	}
	else if (StringOps::StartsWith(Input, "0x"))
		base = 16;

	//todo: handle invalid numbers: 0xff_8; 99_8;

	if (Input.find('.') != Input.npos)
		return { InstructionType::Float, std::stod(Input) };
	else
		return { InstructionType::Integer, std::stoll(Input, nullptr, base) };
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
	CreateOperator("!", Notation::Prefix , Association::None, 3);
	CreateOperator("?", Notation::Postfix, Association::None, 7);
	CreateOperator(",", Notation::Infix, Association::LeftToRight, 8);
	CreateOperator("+", Notation::Infix, Association::RightToLeft, 5);
	CreateOperator("*", Notation::Infix, Association::RightToLeft, 4);
	CreateOperator("=", Notation::Infix, Association::RightToLeft, 10);
	CreateOperator(":", Notation::Infix, Association::RightToLeft, 9);
	CreateOperator("in", Notation::Infix, Association::RightToLeft, 10);
}
