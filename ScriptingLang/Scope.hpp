#include "pch.hpp"

using Id = std::string;
using Value = int;

Value Undefined = 0;

class Scope
{
public:
	Scope() : parent(nullptr) { }
	Scope(Scope* Parent) : parent(Parent) { }

	inline Value& Set(const Id& Name, const Value& Value)
	{
		values[Name] = Value;
		return values[Name];
	}

	Value& Get(const Id& Name, bool SearchParents = true);
	bool Has(const Id& Name, bool SearchParents = true);

	bool Remove(const Id& Name);

protected:
	std::map<Id, Value> values;
	Scope* parent;
};