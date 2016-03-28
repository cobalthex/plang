#include "pch.hpp"
#include "Construct.hpp"

namespace Plang
{
    class Scope
    {
    public:
    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }

		Reference& Set(const std::string& Name, const Construct& Cxt, bool SearchParents = false);
		Reference& Set(const std::string& Name, const Reference& Ref, bool SearchParents = false);
    	Reference& Get(const std::string& Name, bool SearchParents = true);
        Scope* Where(const std::string& Name); //which scope a property is defined in
    	bool Has(const std::string& Name, bool SearchParents = true);

    	bool Remove(const std::string& Name);

    protected:
        std::map<std::string, Reference> properties;
		std::vector<Reference> indices;
    	Scope* parent;

		friend std::ostream& operator << (std::ostream& Stream, const Scope& Scope);
    };
};