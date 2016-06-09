#pragma once

#include "pch.hpp"
#include "variant.hpp"
#include "Location.hpp"
#include "types.hpp"

namespace Plang
{
	enum class InstructionType : size_t
	{
		Unknown,
		Program,

		Statement,
		Expression, //functions
		Call, //call to an expression
		Operation, //A call to an operator (x + y) -- simplified node structure
		ControlStructure, //a control structure, such as an if statement or loop

		Block,
		Identifier,
		Accessor,

		Integer,
		Float,
		Decimal,
		String,

		Tuple,
		List,
		Array
	};

	/*node structure for various instruction types:

		Expression:
			tuple
				** (Arguments)
			block
				** (Body)
	
		Call:
			* (function: can be accessor, identifier, etc)
			tuple
				** (Arguments)

		Operation: (id)
			* (left side argument)
			* (right side argument)

		ControlStructure: (id)
			* (condition: can be call/operation)
			block
				** (body)
	*/

	class Instruction
	{
	public:
		Instruction() : type(InstructionType::Unknown) { }
		Instruction(InstructionType Type) : type(Type) { }
		Instruction(InstructionType Type, IntT Value) : type(Type), value(Value) { }
		Instruction(InstructionType Type, FloatT Value) : type(Type), value(Value) { }
		Instruction(InstructionType Type, StringT Value) : type(Type), value(Value) { }

		InstructionType type;
		mapbox::util::variant<IntT, FloatT, StringT> value;

		operator std::string() const;
	};

	struct SyntaxTreeNode
	{
		SyntaxTreeNode() = default;
		SyntaxTreeNode(const Instruction& Instruction, SyntaxTreeNode* Parent, const Location& Location)
			: instruction(Instruction), parent(Parent), children(), location(Location) { }

		Instruction instruction;

		SyntaxTreeNode* parent; //nullptr if top level
		std::vector<SyntaxTreeNode> children;

		Location location;
	};

	class SyntaxTree
	{
	public:
		SyntaxTree() : root(InstructionType::Program, nullptr, Location()) { }

		SyntaxTreeNode root;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::SyntaxTree& SyntaxTree);
std::ostream& operator << (std::ostream& Stream, const Plang::SyntaxTreeNode& SyntaxTreeNode);

//todo: proper string deconstruction (possibly store all strings in a set)
