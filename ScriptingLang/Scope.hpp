#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Reference.hpp"

namespace Plang
{
	//A single scope. It can inherit from other scopes
	//By default, lexical scopes will set known properties where defined and dot scopes will set on the deepest scope
    class Scope
    {
    public:
    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }

		AnyRef& Set(const StringT& Name, const AnyRef& Ref, bool SearchParents = false); //If SearchParents is true, property is set where it is defined or in this scope if not found
    	AnyRef& Get(const StringT& Name, bool SearchParents = true);
		inline const AnyRef& Get(const StringT& Name, bool SearchParents = true) const { return Get(Name, SearchParents); }
        Scope* Where(const StringT& Name); //which scope a property is defined in
    	bool Has(const StringT& Name, bool SearchParents = true) const;

    	bool Remove(const StringT& Name);

		Scope& Merge(const Scope& Other, bool Overwrite = true); //Merge another scope into this one (does not include inherited properties)

    protected:
		Scope* parent;
        std::map<StringT, AnyRef> variables;

		friend std::ostream& operator << (std::ostream& Stream, const Scope& Scope);
    };
};