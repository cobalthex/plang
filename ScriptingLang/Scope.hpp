#include "pch.hpp"
#include "Handle.hpp"

namespace Plang
{
	class Construct;

    class Scope
    {
    public:
    	Scope() : parent(nullptr) { }
    	Scope(Scope* Parent) : parent(Parent) { }

		//Reference& Set(const std::string& Name, const Reference& Ref, bool SearchParents = false);
    	//Reference& Get(const std::string& Name, bool SearchParents = true);
        //Scope* Where(const std::string& Name); //which scope a value is defined in
    	//bool Has(const std::string& Name, bool SearchParents = true);

    	//bool Remove(const std::string& Name);

    protected:
        std::map<std::string, Construct*> properties;
    	Scope* parent;
    };
};
