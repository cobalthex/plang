#include "pch.hpp"
#include "Construct.hpp"

std::string Plang::TypeToString(const Plang::ConstructType& Type)
{
	switch (Type)
	{
	case Plang::ConstructType::Construct:
		return "Construct";
	case Plang::ConstructType::Bool:
		return "Bool";
	case Plang::ConstructType::Int:
		return "Int";
	case Plang::ConstructType::Float:
		return "Float";
	case Plang::ConstructType::String:
		return "String";
	case Plang::ConstructType::Tuple:
		return "Tuple";
	case Plang::ConstructType::Array:
		return "Array";
	case Plang::ConstructType::List:
		return "List";
	case Plang::ConstructType::Function:
		return "Function";
	case Plang::ConstructType::Script:
		return "Script";
	default:
		return "Unknown";
	}
}

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct)
{
	Stream << TypeToString(Construct.Type()) << "(" << Construct.ToString() << ")";
	return Stream;
}

Plang::Signature::Signature(const ::Array<Argument> Arguments)
	: arguments(Arguments), nSingles(0)
{
	for (size_t i = 0; i < arguments.Length(); i++)
	{
		if (arguments[i].type == ArgumentType::Tuple)
			nSingles++;
	}
	nTuples = arguments.Length() - nSingles;
}

Plang::Scope Plang::Signature::Parse(const Plang::Tuple& Arguments)
{
	Scope s;

	auto nTupleArgs = Arguments.Length() - nSingles;
	auto nArgsPerTuple = nTuples > 0 ? (1 + ((nTupleArgs - 1) / nTuples)) : 0;

	for (size_t i = 0; i < std::min(arguments.Length(), Arguments.Length());)
	{
		auto& prop = Arguments.properties.Get(arguments[i].name);
		if (prop != Undefined)
			s.Set(arguments[i].name, prop);
		else if (arguments[i].type == ArgumentType::Tuple)
		{
			size_t nArgs = std::min(nArgsPerTuple, Arguments.value.Length() - i);
			if (nArgs > 0)
			{
				Reference<Tuple> tup (Arguments.value.Slice(i, nArgs));
				s.Set(arguments[i].name, tup);
				i += nArgs;
			}
		}
		else
		{
			s.Set(arguments[i].name, Arguments.Get(i));
			i++;
		}
	}

	return s;
}

Plang::AnyRef Plang::Function::Call(const Plang::Tuple& Arguments, Plang::Scope* LexScope)
{
	AnyRef rval;

	auto s = signature.Parse(Arguments);
	s.parent = LexScope;

	function(s);

	return rval;
}

struct Frame
{
	Plang::SyntaxTreeNode* node; //the node to evaluate
	size_t index; //index of current instruction
};

Plang::AnyRef Plang::Script::Evaluate(const Plang::Tuple& Arguments, Plang::Scope* LexScope)
{
	if (node == nullptr)
		return Undefined;

	std::vector<AnyRef> registers; //holds intermediate values for functions

	auto scope = signature.Parse(Arguments);
	scope.parent = LexScope;

	std::stack<Frame> stack;
	stack.push({ node, 0 });
	while (!stack.empty())
	{
		auto top = stack.top();
		stack.pop();

		//todo: handle : and = somehow

		bool nest = false;
		for (size_t i = top.index; i < top.node->children.size(); i++)
		{
			auto& child = top.node->children[i];

			if (child.instruction.type == InstructionType::Accessor)
				; //todo
			if (child.instruction.type == InstructionType::Integer)
				registers.push_back(Reference<Int>(child.instruction.value.get<Int::ValueType>()));
			else if (child.instruction.type == InstructionType::Float)
				registers.push_back(Reference<Float>(child.instruction.value.get<Float::ValueType>()));
			else if (child.instruction.type == InstructionType::String)
				registers.push_back(Reference<String>(child.instruction.value.get<String::ValueType>()));
			else if (child.instruction.type == InstructionType::Identifier)
				registers.push_back(scope.Get(child.instruction.value.get<StringT>()));
			else if (child.children.size() > 0)
			{
				stack.push({ &child, 0 });
				nest = true;
				i++;
				break;
			}
		}

		if (!nest)
		{
			auto& inst = top.node->instruction;
			if (inst.type == InstructionType::Tuple)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				Plang::Tuple tup(::Array<AnyRef>(registers.data() + start, len));
				registers.erase(registers.begin() + start, registers.begin() + start + len);
				registers.push_back(tup);
			}
			else if (inst.type == InstructionType::Array)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				Plang::Array tup(::Array<AnyRef>(registers.data() + start, len));
				registers.erase(registers.begin() + start, registers.begin() + start + len);
				registers.push_back(tup);
			}
			else if (inst.type == InstructionType::Call)
			{

				auto& fn = scope.Get(inst.value.get<StringT>());

				auto len = top.node->children.size();
				auto start = registers.size() - len;
				Tuple tup (::Array<AnyRef>(registers.data() + start, len));
				registers.erase(registers.begin() + start, registers.begin() + start + len);

				if (fn == Undefined)
					throw inst.value.get<StringT>() + " is undefined and not callable";
				else if (fn->Type() == ConstructType::Script)
					registers.push_back(Reference<Script>(fn)->Evaluate(tup, &scope));
				else if (fn->Type() == ConstructType::Function)
					registers.push_back(Reference<Function>(fn)->Call(tup, &scope));
				else
					throw "not callable";
			}
			//any other instructions can be ignored (todo: verify)
		}
	}

	return Undefined;
}
