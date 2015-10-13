#pragma once

#include "pch.hpp"
#include "Union.hpp"

namespace Plang
{
	using Int = int64_t;
	using Float = double;
	using String = std::string;

	enum class InstructionType : size_t
	{
		Unknown,

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
		Array,

		Expression, //functions, control statements
		Call, //call to an expression
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

		Int& AsInt() { return value.As<Int>(); }
		const Int& AsInt() const { return value.As<Int>(); }

		Float& AsFloat() { return value.As<Float>(); }
		const Float& AsFloat() const { return value.As<Float>(); }

		String& AsString() { return value.As<String>(); }
		const String& AsString() const { return value.As<String>(); }

	private:
		Union<Int, Float, String> value;
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
		SyntaxTree()
			: root({ InstructionType::Block }) { }

		SyntaxTreeNode root;
	};
};

std::ostream& operator << (std::ostream& Stream, Plang::SyntaxTree& SyntaxTree);

//todo: proper string deconstruction (possibly store all strings in a set)