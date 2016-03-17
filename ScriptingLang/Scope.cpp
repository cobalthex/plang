#include "pch.hpp"
#include "Scope.hpp"

Plang::Reference& Plang::Scope::Set(const Plang::String& Name, const Plang::Reference& Ref, bool SearchParents)
{
	auto scope = this;
	if (SearchParents)
		scope = Where(Name);

	return (scope->variables[Name] = Ref);
}

Plang::Reference& Plang::Scope::Get(const Plang::String& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		auto value = scope->variables.find(Name);
		if (value != scope->variables.end())
			return value->second;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return Plang::Reference::Undefined;
}

Plang::Scope* Plang::Scope::Where(const Plang::String& Name)
{
	auto scope = this;

	do
	{
		if (scope->variables.find(Name) != scope->variables.end())
			break;

		scope = scope->parent;
	} while (scope != nullptr);

	return scope;
}

bool Plang::Scope::Has(const Plang::String& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		if (scope->variables.find(Name) != scope->variables.end())
			return true;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return false;
}

bool Plang::Scope::Remove(const Plang::String& Name)
{
	auto value = variables.find(Name);
	if (value != variables.end())
	{
		variables.erase(value);
		return true;
	}
	return false;
}
