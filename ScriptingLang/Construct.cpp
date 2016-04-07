#include "pch.hpp"
#include "Construct.hpp"

std::string Plang::TypeToString(const Plang::ValueType& Type)
{
	switch (Type)
	{
	case Plang::ValueType::Bool:
		return "Bool";
	case Plang::ValueType::Int:
		return "Int";
	case Plang::ValueType::Float:
		return "Float";
	case Plang::ValueType::String:
		return "String";
	case Plang::ValueType::Array:
		return "Array";
	default:
		return "Unknown";
	}
}

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct)
{
	Stream << "Type: " << TypeToString(Construct.Type());
	//properties
	//indices

	return Stream;
}

Plang::Construct::operator std::string() const
{
	return "[Object]";
}
