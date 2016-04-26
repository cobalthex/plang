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
	case Plang::ConstructType::Array:
		return "Array";
	case Plang::ConstructType::Script:
		return "Script";
	case Plang::ConstructType::Function:
		return "Function";
	case Plang::ConstructType::ScriptFunction:
		return "ScriptFunction";
	default:
		return "Unknown";
	}
}

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct)
{
	Stream << TypeToString(Construct.Type()) << "(" << Construct.ToString() << ")";
	return Stream;
}

Plang::Scope Plang::Signature::Parse(const Tuple& Arguments)
{
	Scope s;

	size_t nTuples = 0;
	for (size_t i = 0; i < signature.Length(); i++)
	{
		if (signature[i].type == ArgumentType::Tuple)
			nTuples++;
	}

	size_t nTupleArgs = Arguments.Length() - nTuples;
	size_t nArgsPerTuple = (size_t)ceil((float)nTuples / nTupleArgs);

	for (size_t i = 0; i < Arguments.value.Length();)
	{
		if (signature[i].type == ArgumentType::Tuple)
		{
			size_t nArgs = std::min(nTupleArgs, Arguments.value.Length() - i);
			if (nArgs > 0)
			{
				Reference<Tuple> tup (Arguments.value.Slice(i, nArgs));
				s.Set(signature[i].name, tup);
				i += nArgs;
			}
		}
		else
		{
			s.Set(signature[i].name, Arguments.Get(i));
			i++;
		}
	}

	return s;
}

Plang::AnyRef Plang::Function::Call(Scope* LexScope, const Tuple& Arguments)
{
	AnyRef rval;

	auto s = signature.Parse(Arguments);
	s.parent = LexScope;

	//call fn

	return rval;
}

Plang::AnyRef Plang::Script::Evaluate(Plang::Scope* LexScope, const Tuple& Arguments)
{
	AnyRef rval;

	auto s = signature.Parse(Arguments);
	s.parent = LexScope;

	//eval script

	return rval;
}
