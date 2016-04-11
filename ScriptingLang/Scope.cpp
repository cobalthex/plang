#include "pch.hpp"
#include "Scope.hpp"
#include "Construct.hpp"

Plang::AnyRef& Plang::Scope::Set(const Plang::StringT& Name, const Plang::AnyRef& Ref, bool SearchParents)
{
	auto scope = this;
	if (SearchParents)
		scope = Where(Name);

	return (scope->variables[Name] = Ref);
}

Plang::AnyRef& Plang::Scope::Get(const Plang::StringT& Name, bool SearchParents)
{
	auto scope = this;

	do
	{
		auto value = scope->variables.find(Name);
		if (value != scope->variables.end())
			return value->second;

		scope = scope->parent;
	} while (scope != nullptr && SearchParents);

	return Plang::Undefined;
}


Plang::Scope* Plang::Scope::Where(const Plang::StringT& Name)
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

bool Plang::Scope::Has(const Plang::StringT& Name, bool SearchParents) const
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

bool Plang::Scope::Remove(const Plang::StringT& Name)
{
	auto value = variables.find(Name);
	if (value != variables.end())
	{
		variables.erase(value);
		return true;
	}
	return false;
}

std::ostream & Plang::operator<<(std::ostream & Stream, const Scope& Scope)
{
	std::cout << "Parent: " << std::hex << std::showbase << Scope.parent << std::endl;
	std::cout << "Variables:" << std::endl;
	for (auto& prop : Scope.variables)
	{
		std::cout << "  " << prop.first << ": " << prop.second->ToString() << std::endl;
	}
	std::cout << std::endl;
	return Stream;
}
