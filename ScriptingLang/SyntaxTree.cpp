#include "pch.hpp"
#include "SyntaxTree.hpp"

static const std::string types[] =
{
	"Unknown",
	"Block",
	"Identifier",
	"Accessor",
	"Integer",
	"Float",
	//"Decimal",
	"String",
	"Tuple",
	"NamedTuple",
	"List",
	"Array",
	"Expression"
};

void PrintDepth(std::ostream& Stream, size_t Depth, const Plang::SyntaxTreeNode& Node)
{
	Stream << std::string(Depth * 4, ' ') << std::left << std::setw(10) << types[(size_t)Node.instruction.type] << "  ";
	if (Node.instruction.type == Plang::InstructionType::Integer)
		Stream << Node.instruction.AsInt() << std::endl;
	else if (Node.instruction.type == Plang::InstructionType::Float)
		Stream << Node.instruction.AsFloat() << std::endl;
	else if (Node.instruction.type == Plang::InstructionType::String || Node.instruction.type == Plang::InstructionType::Identifier)
		Stream << Node.instruction.AsString() << std::endl;
	else
		Stream << std::endl;

	for (auto& i : Node.children)
		PrintDepth(Stream, Depth + 1, i);
}

std::ostream& operator << (std::ostream& Stream, Plang::SyntaxTree& SyntaxTree)
{
	PrintDepth(Stream, 0, SyntaxTree.root);
	return Stream;
}