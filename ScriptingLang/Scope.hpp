#include "pch.hpp"
#include "Value.hpp"

namespace Plang
{
    class Scope
    {
    public:
    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }

    	Reference& Set(const String& Name, const Reference& Ref, bool SearchParents = false);
    	Reference& Get(const String& Name, bool SearchParents = true);
        Scope* Where(const String& Name); //where is a value defined
    	bool Has(const String& Name, bool SearchParents = true);

    	bool Remove(const String& Name);

    protected:
        Dictionary variables;
    	Scope* parent;
    };
};
