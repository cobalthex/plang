#include "pch.hpp"
#include "Reference.hpp"

namespace Plang
{
    class Scope
    {
    public:
    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }

    	Reference& Set(const Reference& Name, const Reference& Ref, bool SearchParents = false);
    	Reference& Get(const Reference& Name, bool SearchParents = true);
        Scope* Where(const Reference& Name); //which scope a value is defined in
    	bool Has(const Reference& Name, bool SearchParents = true);

    	bool Remove(const Reference& Name);

    protected:
        std::map<Reference, Reference> properties;
    	Scope* parent;
    };
};
