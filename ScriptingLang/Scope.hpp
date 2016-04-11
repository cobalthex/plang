#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Reference.hpp"

namespace Plang
{
    class Scope
    {
    public:
    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }

		AnyRef& Set(const StringT& Name, const AnyRef& Ref, bool SearchParents = false);
    	AnyRef& Get(const StringT& Name, bool SearchParents = true);
		inline const AnyRef& Get(const StringT& Name, bool SearchParents = true) const { return Get(Name, SearchParents); }
        Scope* Where(const StringT& Name); //which scope a property is defined in
    	bool Has(const StringT& Name, bool SearchParents = true) const;

    	bool Remove(const StringT& Name);

    protected:
		Scope* parent;
        std::map<StringT, AnyRef> variables;

		friend std::ostream& operator << (std::ostream& Stream, const Scope& Scope);
    };
};