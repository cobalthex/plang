#pragma once

#include "pch.hpp"
#include "variant.hpp"

namespace Plang
{
	using Int = int64_t;
	using Float = double;
	using String = std::string;

	enum class InstructionType : size_t
	{
		Unknown,
		Program,

		Statement,
		Expression, //functions, control statements
		Call, //call to an expression

		Block,
		Identifier,
		Accessor,

		Integer,
		Float,
		//Decimal,
		String,

		Tuple,
		NamedTuple,

		List,
		Array
	};
	class Instruction
	{
	public:
		Instruction() : type(InstructionType::Unknown) { }
		Instruction(InstructionType Type) : type(Type) { }
		Instruction(InstructionType Type, Int Value) : type(Type), value(Value) { }
		Instruction(InstructionType Type, Float Value) : type(Type), value(Value) { }
		Instruction(InstructionType Type, String Value) : type(Type), value(Value) { }

		InstructionType type;
		mapbox::util::variant<Int, Float, String> value;
	};

	struct SyntaxTreeNode
	{
		SyntaxTreeNode() = default;
		SyntaxTreeNode(const Instruction& Instruction, SyntaxTreeNode* Parent = nullptr)
			: instruction(Instruction), parent(Parent), children() { }

		Instruction instruction;

		SyntaxTreeNode* parent; //nullptr if top level
		std::vector<SyntaxTreeNode> children;
	};

	class SyntaxTree
	{
	public:
		SyntaxTree() { root.instruction.type = InstructionType::Program; }

		SyntaxTreeNode root;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::SyntaxTree& SyntaxTree);
std::ostream& operator << (std::ostream& Stream, const Plang::SyntaxTreeNode& SyntaxTreeNode);
std::ostream& operator << (std::ostream& Stream, const Plang::Instruction& Instruction);

//todo: proper string deconstruction (possibly store all strings in a set)
