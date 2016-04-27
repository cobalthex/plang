#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Reference.hpp"
#include "Array.hpp"

namespace Plang
{
	//A single scope. It can inherit from other scopes
	//By default, lexical scopes will set known properties where defined and dot scopes will set on the deepest scope
    class Scope
    {
    public:

    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }
		Scope(const ::Array<std::pair<StringT, AnyRef>>& Variables) : variables(Variables.Data(), Variables.Data() + Variables.Length()) { }

		AnyRef& Set(const StringT& Name, const AnyRef& Ref, bool SearchParents = false); //If SearchParents is true, property is set where it is defined or in this scope if not found
    	AnyRef& Get(const StringT& Name, bool SearchParents = true);
		const AnyRef& Get(const StringT& Name, bool SearchParents = true) const;
        Scope* Where(const StringT& Name); //which scope a property is defined in
    	bool Has(const StringT& Name, bool SearchParents = true) const;
		inline size_t Count() const { return variables.size(); } //The number of defined variables in this scope
    	bool Remove(const StringT& Name);

		Scope& Merge(const Scope& Other, bool Overwrite = true); //Merge another scope into this one (does not include inherited properties)

		Scope* parent;

    protected:
        std::map<StringT, AnyRef> variables;

		friend std::ostream& operator << (std::ostream& Stream, const Scope& Scope);
    };
};
