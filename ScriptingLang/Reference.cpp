#include "pch.hpp"
#include "Reference.hpp"
#include "Value.hpp"

using namespace Plang;

Reference Reference::Undefined (Value::Undefined);

Reference::Reference(Value* Value)
	: value(Value)
{
	if (value != nullptr)
		value->AddRef();
}
Reference::Reference(const Reference& Reference)
	: value(Reference.value)
{
	if (value != nullptr)
		value->AddRef();
}
Reference::Reference(Reference&& Reference) : value(std::move(Reference.value)) { }

Reference::~Reference()
{
	if (value != nullptr)
		value->FreeRef();
}

Reference& Reference::operator = (const Reference& Reference)
{
	if (this != &Reference)
	{
		value = Reference.value;
		if (value != nullptr)
			value->AddRef();
	}
	return *this;
}
Reference& Reference::operator = (Reference&& Reference)
{
	if (this != &Reference)
		value = std::move(Reference.value);
	return *this;
}