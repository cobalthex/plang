#include "pch.hpp"
#include "Scope.hpp"

Plang::Reference& Plang::Scope::Set(const Plang::Reference& Name, const Plang::Reference& Ref, bool SearchParents)
{
	auto scope = this;
	if (SearchParents)
		scope = Where(Name);

	return (scope->properties[Name] = Ref);
}

Plang::Reference& Plang::Scope::Get(const Plang::Reference& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		auto value = scope->properties.find(Name);
		if (value != scope->properties.end())
			return value->second;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return Plang::Reference::Undefined;
}

Plang::Scope* Plang::Scope::Where(const Plang::Reference& Name)
{
	auto scope = this;

	do
	{
		if (scope->properties.find(Name) != scope->properties.end())
			break;

		scope = scope->parent;
	} while (scope != nullptr);

	return scope;
}

bool Plang::Scope::Has(const Plang::Reference& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		if (scope->properties.find(Name) != scope->properties.end())
			return true;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return false;
}

bool Plang::Scope::Remove(const Plang::Reference& Name)
{
	auto value = properties.find(Name);
	if (value != properties.end())
	{
		properties.erase(value);
		return true;
	}
	return false;
}
