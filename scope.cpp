#include <iostream>
#include <map>
#include <string>

using Id = std::string;
using Value = int;

Value Undefined = 0;

class Scope
{
public:
  Scope() : parent(nullptr) { }
  Scope(Scope* Parent) : parent(Parent) { }

  Value& Set(const Id& Name, const Value& Value)
  {
    values[Name] = Value;
    return values[Name];
  }

  Value& Get(const Id& Name, bool SearchParents = true)
  {
    auto scope = this;

    do
    {
      auto value = scope->values.find(Name);
      if (value != scope->values.end())
        return value->second;

      scope = scope->parent;
    } while (scope != nullptr && SearchParents);

    return Undefined;
  }

  bool Has(const Id& Name, bool SearchParents = true)
  {
    auto scope = this;

    do
    {
      if (scope->values.find(Name) != scope->values.end())
        return true;

      scope = scope->parent;
    } while (scope != nullptr && SearchParents);

    return false;
  }

  std::map<Id, Value> values;
  Scope* parent;
};

int main()
{
  Scope parent;
  Scope child (&parent);

  parent.Set("x", 5);
  parent.Set("y", 6);
  child.Set("x", 7);
  child.Set("z", 8);

  std::cout << child.Get("x") <<
  std::endl << child.Get("y") <<
  std::endl << child.Get("z") << std::endl << std::endl;

  std::cout << child.Has("x") <<
  std::endl << child.Has("x", false) <<
  std::endl << child.Has("y") <<
  std::endl << child.Has("y", false) << std::endl;

  return 0;
}
