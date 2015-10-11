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

Parser::Parser(const Lexer& Lex)
{
	CreatePredefinedExpressions();

	SyntaxTreeNode* parent = &syntaxTree.root;

	std::stack<std::string> blocks; //block matching (mainly for tuples)

	for (auto i = Lex.tokens.begin(); i != Lex.tokens.end(); i++)
	{
		auto& tok = *i;

		if (tok.type == LexerTokenType::PreprocessCmd)
		{
			//all arguments are directly following
		}
		else if (tok.type == LexerTokenType::Number)
		{
			auto number = ParseNumber(tok.value);
			if (number.type == InstructionType::Integer)
				parent->children.push_back({ { number.type, number.value.As<Int>() }, parent });
			else if (number.type == InstructionType::Float)
				parent->children.push_back({ { number.type, number.value.As<Float>() }, parent });
		}
		else if (tok.type == LexerTokenType::String)
			parent->children.push_back({ { InstructionType::String, ParseString(tok.value) }, parent });
		else if (tok.type == LexerTokenType::Terminator)
		{
			//create a nested tuple (and retroactively apply to previous set leading up to a tuple)
			if (parent->instruction.type == InstructionType::Tuple)
			{
				if (blocks.size() > 0 && blocks.top() == "(") // handle unbounded tuples
					assert(false); //todo: Coming later
				else if (blocks.size() < 1)
					parent = parent->parent;

				//todo: lists, arrays
			}
			//
			else if (parent->instruction.type == InstructionType::Identifier)
			{
			}
		}
		else if (tok.type == LexerTokenType::Separator)
		{
			if (parent->instruction.type != InstructionType::Tuple)
			{
				//save previous
				auto previous = parent->children.back();
				parent->children.pop_back();

				parent->children.push_back({ { InstructionType::Tuple }, parent });
				parent = &parent->children.back();

				//move previous item to tuple
				parent->children.push_back(previous);
				previous.parent = parent;
			}
		}
		//chains all acccessors: a.b.c.d => accessor { a, b, c, d }
		else if (tok.type == LexerTokenType::Accessor)
		{
			if (parent->instruction.type != InstructionType::Accessor)
			{
				if (parent->children.size() > 0)
				{
					auto previous = parent->children.back();
					parent->children.pop_back();

					parent->children.push_back({ { InstructionType::Accessor }, parent });
					parent = &parent->children.back();
					
					previous.parent = parent;
					parent->children.push_back(previous);
				}
				else
				{
					parent->children.push_back({ { InstructionType::Accessor }, parent });
					parent = &parent->children.back();
				}
			}
			i++;
			//read all following accessors
			while (i != Lex.tokens.end())
			{
				if (i->type == LexerTokenType::Accessor)
				{
					parent->children.push_back({ { InstructionType::Identifier, "" }, parent }); //multidot properties, .., ...
					i++;
				}
				else
				{
					if (i->type == LexerTokenType::Identifier)
						parent->children.push_back({ { InstructionType::Identifier, i->value }, parent });
					else if (i->type == LexerTokenType::String)
						parent->children.push_back({ { InstructionType::String, ParseString(i->value) }, parent });
					else if (i->type == LexerTokenType::Number)
					{
						auto number = ParseNumber(i->value);

						if (number.type == InstructionType::Integer)
							parent->children.push_back({ { number.type, number.value.As<Int>() }, parent });
						else if (number.type == InstructionType::Float)
							parent->children.push_back({ { number.type, number.value.As<Float>() }, parent });
					}
					else
						break;

					i++;
					if (i->type != LexerTokenType::Accessor)
						break;
					i++;
				}
			}
			parent = parent->parent;
		}
		else if (tok.type == LexerTokenType::RegionOpen)
		{
			blocks.push(tok.value);

			InstructionType it;
			if (tok.value == "(")
				it = InstructionType::Tuple;
			else if (tok.value == "[")
				it = InstructionType::List;
			else if (tok.value == "[|")
				it = InstructionType::Array;
			else if (tok.value == "{")
			{
				it = InstructionType::Block;

				//test if expression
				auto& children = parent->children;
				if (children.size() > 0 && (children.back().instruction.type == InstructionType::Tuple || children.back().instruction.type == InstructionType::NamedTuple))
				{
					SyntaxTreeNode node = { { InstructionType::Expression }, parent };

					//todo: handle precidence for things like a + () { }
					
					size_t strip = (children.size() > 1 && children[children.size() - 2].instruction.type == InstructionType::Identifier ? 2 : 1);
					std::move(children.end() - strip, children.end(), std::back_inserter(node.children));
					children.erase(children.end() - strip, children.end());
				
					node.children.emplace_back(it, &node);
					parent->children.push_back(node);
					parent = &parent->children.back().children.back();
					Reparent(parent, parent->parent);

					continue;
				}
			}
			parent->children.emplace_back(it, parent);
			parent = &parent->children.back();
		}
		else if (tok.type == LexerTokenType::RegionClose)
		{
			//can only close inner-most region
			if (parent->instruction.type == InstructionType::Tuple || parent->instruction.type == InstructionType::NamedTuple)
				assert(tok.value == ")");
			if (parent->instruction.type == InstructionType::List)
				assert(tok.value == "]");
			if (parent->instruction.type == InstructionType::Array)
				assert(tok.value == "|]");
			if (parent->instruction.type == InstructionType::Block)
				assert(tok.value == "}");

			blocks.pop();
			parent = parent->parent;
		}
		else if (tok.type == LexerTokenType::Identifier)
		{
			parent->children.push_back({ { InstructionType::Identifier, tok.value }, parent });
		}
	}
	//second pass to evaluate callables (tuples are deconstructed), and further eliminate any single value tuples
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

void Parser::CreatePredefinedExpressions()
{
	ParseExpression pi;

	pi.name = "+";
	pi.association = Association::LeftToRight;
	pi.precedence = 5;
	predefinedExpressions[pi.name] = pi;

	pi.name = "*";
	pi.association = Association::LeftToRight;
	pi.precedence = 4;
	predefinedExpressions[pi.name] = pi;

	pi.name = "=";
	pi.association = Association::RightToLeft;
	pi.precedence = 15;
	predefinedExpressions[pi.name] = pi;
}