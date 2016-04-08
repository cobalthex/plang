#include "pch.hpp"
#include "Reference.hpp"
#include "Construct.hpp"

Plang::Reference Plang::Reference::Undefined;

std::ostream& operator<<(std::ostream& Stream, const Plang::Reference& Reference)
{
	Stream << Plang::TypeToString(Reference->Type()) << "(" << ((std::string)*Reference) << ")";
	return Stream;
}
