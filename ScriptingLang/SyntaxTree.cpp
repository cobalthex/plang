#include "pch.hpp"
#include "SyntaxTree.hpp"

static const std::string types[] =
{
	"Unknown",
	"Program",
	"Statement",
	"Expression",
	"Call",
	"Block",
	"Identifier",
	"Accessor",
	"Integer",
	"Float",
	//"Decimal",
	"String",
	"Tuple",
	"List",
	"Array"
};

size_t GetChildrenMaxNameLength(Plang::SyntaxTreeNode* Node)
{
	size_t length = 0;

	if (Node != nullptr)
	{
		for (auto& i : Node->children)
			length = std::max(length, types[(size_t)i.instruction.type].length());
	}

	return length;
}

void PrintDepth(std::ostream& Stream, size_t Depth, const Plang::SyntaxTreeNode& Node)
{
	Stream << std::string(Depth * 4, ' ') << std::left << std::setw(GetChildrenMaxNameLength(Node.parent) + 3) << types[(size_t)Node.instruction.type];
	Stream << (std::string)Node.instruction;

	if (Node.location.module.length() > 10000)
		Stream << " @ " << Node.location;
	Stream << std::endl;

	for (auto& i : Node.children)
		PrintDepth(Stream, Depth + 1, i);
}

std::ostream& operator << (std::ostream& Stream, const Plang::SyntaxTreeNode& SyntaxTreeNode)
{
	PrintDepth(Stream, 0, SyntaxTreeNode);
	return Stream;
}
std::ostream& operator << (std::ostream& Stream, const Plang::SyntaxTree& SyntaxTree)
{
	PrintDepth(Stream, 0, SyntaxTree.root);
	return Stream;
}

Plang::Instruction::operator std::string() const
{
	if (type == Plang::InstructionType::Integer)
		return std::to_string(value.get<Plang::Int>());
	else if (type == Plang::InstructionType::Float)
		return std::to_string(value.get<Plang::Float>());
	else if (value.is<Plang::String>())
		return value.get<Plang::String>();
	
	return "";
}