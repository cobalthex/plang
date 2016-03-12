#include "pch.hpp"

using Id = std::string;
using Value = int;

Value Undefined = 0;

class Scope
{
public:
	Scope() : parent(nullptr) { }
	Scope(Scope* Parent) : parent(Parent) { }

	Value& Set(const Id& Name, const Value& Value, bool SearchParents = false);
	Value& Get(const Id& Name, bool SearchParents = true);
    Scope* Where(const Id& Name); //where is a value defined
	bool Has(const Id& Name, bool SearchParents = true);

	bool Remove(const Id& Name);

protected:
	std::map<Id, Value> values;
	Scope* parent;
};
