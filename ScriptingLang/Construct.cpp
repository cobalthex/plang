#include "pch.hpp"
#include "Construct.hpp"

std::string Plang::TypeToString(const Plang::ConstructType& Type)
{
	switch (Type)
	{
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
	default:
		return "Unknown";
	}
}

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct)
{
	Stream << TypeToString(Construct.Type()) << "(" << Construct.ToString() << ")";
	//properties
	//indices

	return Stream;
}