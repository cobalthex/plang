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

	FloatT nTupleArgs = Arguments.Length() - nSingles;
	size_t nArgsPerTuple = nTuples > 0 ? (size_t)ceil(nTupleArgs / nTuples) : 0;

	for (size_t i = 0; i < Arguments.value.Length();)
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

Plang::AnyRef Plang::Script::Evaluate(const Plang::Tuple& Arguments, Plang::Scope* LexScope)
{
	AnyRef rval;

	auto s = signature.Parse(Arguments);
	s.parent = LexScope;

	//eval script

	return rval;
}
