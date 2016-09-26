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
        Operation, //a + b, etc.
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
		Instruction(InstructionType Type, const mapbox::util::variant<IntT, FloatT, StringT>& Value) : type(Type), value(Value) { }

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
        
        template <typename T>
        T& GetValue() { return instruction.value.get<T>(); }
        
        template <typename T>
        const T& GetValue() const { return instruction.value.get<T>(); }
        
        inline SyntaxTreeNode& AddChild(const SyntaxTreeNode& Node)
        {
            children.push_back(Node);
            return children.back();
        } //todo: use everywhere
        
        inline SyntaxTreeNode& PushChild(const SyntaxTreeNode& Node)
        {
            children.insert(children.begin(), Node);
            return children.front();
        }
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
