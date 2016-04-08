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

		Reference& Set(const StringT& Name, const Reference& Ref, bool SearchParents = false);
    	Reference& Get(const StringT& Name, bool SearchParents = true);
        Scope* Where(const StringT& Name); //which scope a property is defined in
    	bool Has(const StringT& Name, bool SearchParents = true);

    	bool Remove(const StringT& Name);

    protected:
        std::map<StringT, Reference> variables;
    	Scope* parent;

		friend std::ostream& operator << (std::ostream& Stream, const Scope& Scope);
    };
};