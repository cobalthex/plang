#include "pch.hpp"
#include "Scope.hpp"

Value& Scope::Get(const Id& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		auto value = scope->values.find(Name);
		if (value != scope->values.end())
			return value->second;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return Undefined;
}

bool Scope::Has(const Id& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		if (scope->values.find(Name) != scope->values.end())
			return true;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return false;
}

bool Scope::Remove(const Id& Name)
{
	auto value = values.find(Name);
	if (value != values.end())
	{
		values.erase(value);
		return true;
	}
	return false;
}