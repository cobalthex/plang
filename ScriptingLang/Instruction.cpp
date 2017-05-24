#include "pch.hpp"
#include "Instruction.hpp"

static std::string instructionTypeStrings[] =
{
    "Unknown",
    "Program",
    "[Statement]",

    "Int",
    "Float",
    "String",

    "Identifier",
    "Accessor",

    "List",
    "Block",
    "Tuple",

    "Call"
};

//get the length of the type name of the longest child of a node. Used for column alignment
size_t GetChildrenMaxNameLength(const Plang::TList& children)
{
	size_t length = 0;

	for (const auto& child : children)
		length = std::max(length, child.TypeName().length());

	return length;
}


std::string Plang::Instruction::TypeName() const
{
    return instructionTypeStrings[(size_t)type];
}

Plang::Instruction::operator std::string() const
{
    //todo: use visitors
    if (auto pval = std::get_if<nullptr_t>(&value))
        return "[null]";
    if (auto pval = std::get_if<std::string>(&value))
    {
        if (type == InstructionType::Identifier)
            return *pval;
        return '"' + *pval + '"';
    }
    if (auto pval = std::get_if<TInt>(&value))
        return std::to_string(*pval);
    if (auto pval = std::get_if<TFloat>(&value))
        return std::to_string(*pval);
	return "";
}

struct printframe
{
    printframe(const Plang::Instruction& instruction, size_t depth, size_t width)
        : instruction(instruction), depth(depth), width(width) { }

    const Plang::Instruction& instruction;
    size_t depth;
    size_t width;
};
std::ostream& Plang::operator << (std::ostream& stream, const Plang::Instruction& instruction)
{
    std::stack<printframe> prints;
    prints.emplace(instruction, 0, 0);

    while (!prints.empty())
    {
        auto top = prints.top();
        prints.pop();

        stream << std::string(top.depth * 4, ' ') << std::left << std::setw(top.width + 2) << (top.instruction.TypeName() + ": ");
        stream << (std::string)top.instruction;
        //stream << " @ " << top.instruction.location;
        stream << "\n";

        //todo: fix order

        auto&& children = std::get_if<Plang::TList>(&top.instruction.value);
        if (children)
        {
            auto width = GetChildrenMaxNameLength(*children);
            for (auto i = children->crbegin(); i < children->crend(); ++i)
                prints.emplace(*i, top.depth + 1, width);
        }
    }

    return stream;
}