#include "pch.hpp"
#include "Context.hpp"

Plang::AnyRef Plang::Context::Evaluate(const Plang::SyntaxTreeNode* Script)
{
	std::stack<AnyRef> results; //results of inner most calls are pushed here to be used by shallower calls

	for (auto& node : Script->children)
	{
		if (node.instruction.type == InstructionType::Call)
		{
			
		}
	}
}