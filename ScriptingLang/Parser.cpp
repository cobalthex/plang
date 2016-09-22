#include "pch.hpp"
#include "Parser.hpp"
#include "StringOps.hpp"

using namespace Plang;

void PrintTree(SyntaxTreeNode* Node)
{
	if (Node == nullptr)
		std::cout << "** (PrintTree) Node is null **\n";

	while (Node->parent != nullptr)
		Node = Node->parent;
	std::cout << *Node << std::endl;
}

SyntaxTreeNode MakeNode(InstructionType Type, const Location& Location, SyntaxTreeNode* Parent = nullptr, const std::string& Value = "")
{
	return { { Type, Value }, Parent, Location };
}

bool IsIType(const SyntaxTreeNode* Node, InstructionType Type)
{
    assert(Node != nullptr);
    return Node->instruction.type == Type;
}

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
	CreatePredefinitions();

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

//Convert a function call to a control structure (see asserts for requirements) - does not add block
//does not do anything if cannot be converted
//Returns Node
SyntaxTreeNode* ToControlStructure(SyntaxTreeNode* Node)
{
	if (!IsIType(Node, InstructionType::Call)
		|| !IsIType(&Node->children.front(), InstructionType::Identifier)
		|| Node->children.back().children.size() != 1)
		return Node;

	auto condition = std::move(Node->children.back().children.front());
	condition.parent = Node;

	Node->instruction.type = InstructionType::ControlStructure;
	Node->instruction.value = Node->children.front().instruction.value;
	Node->children.clear();
	Node->children.push_back(std::move(condition));

	return Node;
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

		SyntaxTreeNode node(number, parent, Token->location);

		if ((Token != List.cbegin() && (Token - 1)->type == LexerTokenType::Accessor) ||
			(parent->children.size() > 0 && IsIType(&parent->children.back(),InstructionType::Accessor)))
			parent->children.back().children.push_back(node);
		else
			parent->children.push_back(node);
	}
	else if (Token->type == LexerTokenType::String)
	{
		//concatinate adjacent strings
		if (parent->children.size() > 0 && IsIType(&parent->children.back(),InstructionType::String))
			parent->children.back().instruction.value.get<StringT>().append(ParseString(Token->value));
		else
			parent->children.push_back(MakeNode(InstructionType::String, Token->location, parent, ParseString(Token->value)));
	}
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

		if (IsIType(parent, InstructionType::ControlStructure))
		{
			parent = parent->parent;
			ParseStatement(parent);
			parent = parent->parent;
		}

		if (!IsIType(parent, InstructionType::Program)
			&& !IsIType(parent, InstructionType::Block))
			throw ParserException("Invalid terminator", Token->value, Token->location);

		parent->children.push_back({ { InstructionType::Statement }, parent, Token->location });
		parent = &parent->children.back();
	}
	else if (Token->type == LexerTokenType::Separator)
	{
		while (!IsIType(parent, InstructionType::Statement))
			parent = parent->parent;

		ParseStatement(parent);
		parent = parent->parent;

		parent->children.push_back(MakeNode(InstructionType::Statement, Token->location, parent));
		parent = &parent->children.back();
	}
	//chains all acccessors: a.b.c.d => accessor { a, b, c, d }
	else if (Token->type == LexerTokenType::Accessor)
	{
		//todo: z.1.x or z.1.x.y don't work

		if (parent->children.size() > 0 && IsIType(&parent->children.back(), InstructionType::Accessor))
			return;
		else if (Token != List.cbegin() && (Token - 1)->type == LexerTokenType::Accessor)
			parent->children.back().children.push_back(MakeNode(InstructionType::Identifier, Token->location, parent));
		else if (parent->instruction.type != InstructionType::Accessor)
		{
			auto particle = parent->children.back();
			parent->children.pop_back();

			parent->children.push_back(MakeNode(InstructionType::Accessor, Token->location, parent));
			auto& node = parent->children.back();

			node.children.push_back(particle);
			particle.parent = &node;
		}
	}
	else if (Token->type == LexerTokenType::RegionOpen)
	{
        //control structure or block
		if (Token->value == "{")
		{
			if (parent->children.size() > 0)
			{
				//if previous particle is call, convert to control structure
				if (IsIType(&parent->children.back(), InstructionType::Call))
				{
					parent->children.back().instruction.type = InstructionType::ControlStructure;

					parent = &parent->children.back();
				}

				//if previous particle is tuple, convert to expression
				else if (IsIType(&parent->children.back(), InstructionType::Tuple))
				{
					auto tuple = parent->children.back();
					parent->children.pop_back();
					
					parent->children.push_back(MakeNode(InstructionType::Expression, tuple.location, parent));
					
					auto& node = parent->children.back();
					node.children.push_back(tuple);
					tuple.parent = &node;

					parent = &node;
				}
			}

			parent->children.push_back(MakeNode(InstructionType::Block, Token->location, parent));
			parent = &parent->children.back();
		}
		//call
		else if (Token->value == "(" && parent->children.size() > 0)
		{
			auto ident = parent->children.back();
			parent->children.pop_back();

			parent->children.push_back(MakeNode(InstructionType::Call, ident.location, parent));
			parent = &parent->children.back();

			ident.parent = parent;
			parent->children.push_back(ident); //function to call
            parent->children.push_back(MakeNode(InstructionType::Tuple, Token->location, parent)); //arguments
			parent = &parent->children.back();
		}
		else
        {
            auto node = MakeNode(InstructionType::Unknown, Token->location, parent);

			if (Token->value == "(")
				node.instruction.type = InstructionType::Tuple;
            else if (Token->value == "[")
                node.instruction.type = InstructionType::List;
			else if (Token->value == "[|")
				node.instruction.type = InstructionType::Array;
            
            parent->children.push_back(node);
            parent = &parent->children.back();
		}
        
        parent->children.push_back(MakeNode(InstructionType::Statement, Token->location, parent));
		parent = &parent->children.back();
	}
	else if (Token->type == LexerTokenType::RegionClose)
	{
        //todo: correct closing tag per open tag
        
		if (IsIType(parent, InstructionType::Statement) && parent->children.size() < 1)
		{
			parent = parent->parent;
			parent->children.pop_back();
		}
		else
			ParseStatement(parent);

		while (!IsRegion(parent->instruction))
		{
			if (parent->parent == nullptr || IsIType(parent->parent, InstructionType::Program))
				throw ParserException("Unknown region close", Token->value, Token->location);
			parent = parent->parent;
		}

		parent = parent->parent;

		if (IsIType(parent, InstructionType::ControlStructure) && Token->value == ")")
		{
			parent->children.push_back(MakeNode(InstructionType::Block, Token->location, parent));
		}
		else if (IsIType(parent, InstructionType::Call)
				 || IsIType(parent, InstructionType::Expression)
				 || IsIType(parent, InstructionType::ControlStructure))
			parent = parent->parent;
	}
	else if (Token->type == LexerTokenType::Identifier)
	{
		//todo: handle prefix/postfix
		//-a - b <--- if - and op #2 is also op, use prefix

        const auto& op = operators.find(Token->value);
        if (op != operators.end())
        {
            if (op->second.notation == Notation::Prefix &&
                (parent->children.size() < 1 ||
                 (IsIType(&parent->children.back(), InstructionType::Identifier) &&
                  operators.find(parent->children.back().instruction.value.get<std::string>()) != operators.end())))
                std::cout << "moo\n";
        }

        bool isParentAccessor = (parent->children.size() > 0 && IsIType(&parent->children.back(), InstructionType::Accessor));
	
		SyntaxTreeNode node = MakeNode(InstructionType::Identifier, Token->location, parent, Token->value);
		
		bool isLastTokenAccessor = (Token != List.cbegin() && (Token - 1)->type == LexerTokenType::Accessor);
		if (isLastTokenAccessor && isParentAccessor) //part of an accessor
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
		auto temp = Statement->children.front();
        *Statement = std::move(temp);
		Reparent(Statement, _parent);
    }
    else
    {
        //create output/op stacks (shunting yard)
        std::vector<SyntaxTreeNode> output, ops;
        for (auto& i : Statement->children)
        {
            auto op = operators.find(i.instruction.value.is<std::string>() ?
                                     i.instruction.value.get<std::string>() :
                                     "");
            if (op != operators.end())
            {
                //should be while new < top, move top to values
                //move operators over that have higher precedence
                while (ops.size() > 0)
                {
                    auto precedence = operators.find(ops.back().instruction.value.get<std::string>())->second.precedence;

                    if ((op->second.association == Association::LeftToRight && op->second.precedence <= precedence) ||
                        (op->second.association == Association::RightToLeft && op->second.precedence < precedence))
                        break;

                    output.push_back(ops.back());
                    ops.pop_back();
                }
                
                auto oper = MakeNode(InstructionType::Operation, i.location, i.parent);
                oper.children.push_back(i);
                oper.children.push_back(MakeNode(InstructionType::Tuple, i.location));
                ops.push_back(oper);
            }
            else
            {
                //todo: f() + x does not work
                //todo: (f()) + x does not work
                
                //control structures: ctrl (...) ...;
                /*if (output.size() > 0 && IsIType(&output.back(), InstructionType::Call))
                {
                    output.back().instruction.type = InstructionType::ControlStructure;

                    if (IsIType(&i, InstructionType::Block))
                        output.back().children.push_back(i);
                    else
                    {
                        auto node = MakeNode(InstructionType::Block, i.location);
                        node.children.push_back(i);
                        output.back().children.push_back(node);
                    }
                    Reparent(&output.back(), output.back().parent);
                }
                //x[] accessor
                else*/ if (output.size() > 0 && IsIType(&i, InstructionType::List))
                {
                    i.instruction.type = InstructionType::Accessor;
                    
                    if (i.children.size() < 1)
                        throw ParserException("Missing accessor", i.instruction, i.location);
                    
                    i.children.insert(i.children.begin(), std::move(output.back()));
                    Reparent(&i, i.parent);
                    output.back() = std::move(i);
                }
                else
                    output.push_back(i);
            }
        }
        
        //some particles may be combined above
        if (output.size() == 1 && ops.size() == 0)
        {
            *Statement = std::move(output.front());
            Reparent(Statement, _parent);
            return;
        }

        if (ops.size() < 1)
            throw ParserException("Unexpected identifier", output.back().instruction, output.back().location); //where bare words support whould go
        
        //push rest of ops onto output stack
        while (ops.size() > 1)
        {
            output.push_back(ops.back());
            ops.pop_back();
        }

        //create root call
        *Statement = std::move(ops.back());
        Reparent(Statement, _parent);
        _parent = Statement;

        //build syntax tree from output stack
        while (output.size() > 0)
        {
            auto& tup = _parent->children.back();
            //nested operators (remember that outputs are read rtl but operands are in ltr order)
            if (IsIType(&output.back(), InstructionType::Operation))
            {
                tup.children.insert(tup.children.begin(), std::move(output.back()));
                Reparent(&tup.children.front(), &tup);
                _parent = &tup.children.front();
            }
            else
            {
                tup.children.insert(tup.children.begin(), std::move(output.back()));
                Reparent(&tup.children.front(), _parent);
                if (tup.children.size() > 1)
                {
                    //todo: eval intrinsic ops here like # + #

                    _parent = _parent->parent;

                    if (_parent == nullptr)
                        throw ParserException("Error, too many statements", "", output.back().location);
                }
            }

            output.pop_back();
        }
    }
    
    //todo: needs to happen in operator parsing: (x) + 1
    //handle single item tuples (deconstructed to single value)
    if (IsIType(Statement, InstructionType::Tuple) && Statement->children.size() == 1)
    {
        auto temp = Statement->children[0];
        *Statement = std::move(temp);
    }

	//todo: leftover ops should give errors

	//todo: error reporting to allow for finding all errors w/ option to fail fast
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
    assert(!Name.empty());
	operators[Name] = { Name, Notat, Assoc, Precedence };
}
void Parser::CreatePredefinitions()
{
	CreateOperator("!", Notation::Prefix , Association::None, 3);
	CreateOperator("?", Notation::Postfix, Association::None, 7);
	CreateOperator(",", Notation::Infix, Association::LeftToRight, 8);
	CreateOperator("+", Notation::Infix, Association::RightToLeft, 5);
	CreateOperator("*", Notation::Infix, Association::RightToLeft, 4);
	CreateOperator("=", Notation::Infix, Association::RightToLeft, 10);
	CreateOperator(":", Notation::Infix, Association::RightToLeft, 9);
	CreateOperator("in", Notation::Infix, Association::LeftToRight, 10);
	CreateOperator("is", Notation::Infix, Association::None, 10);
	CreateOperator("as", Notation::Infix, Association::LeftToRight, 10);
}
