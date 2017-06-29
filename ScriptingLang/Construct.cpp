#include "pch.hpp"
#include "Construct.hpp"
#include "Parser.hpp"
#include "Lexer.hpp"

Plang::AnyRef Plang::Undefined(nullptr);

std::string Plang::TypeToString(const Plang::ConstructType& Type)
{
    switch (Type)
    {
    case Plang::ConstructType::Construct:
        return "Construct";
    case Plang::ConstructType::Bool:
        return "Bool";
    case Plang::ConstructType::Int:
        return "Int";
    case Plang::ConstructType::Float:
        return "Float";
    case Plang::ConstructType::String:
        return "String";
    case Plang::ConstructType::Tuple:
        return "Tuple";
    case Plang::ConstructType::Array:
        return "Array";
    case Plang::ConstructType::List:
        return "List";
    case Plang::ConstructType::Function:
        return "Function";
    case Plang::ConstructType::Script:
        return "Script";
    default:
        return "Unknown";
    }
}
Plang::AnyRef& Plang::Construct::Set(const std::string& Name, const Plang::AnyRef& Ref, bool SearchParents)
{
	auto scope = this;

	while (scope->prototype != nullptr)
	{
		if (scope->properties.find(Name) != scope->properties.end())
			break;

		scope = scope->prototype.get();
	}

    return (scope->properties[Name] = Ref);
}

Plang::AnyRef& Plang::Construct::Get(const std::string& Name, bool SearchParents)
{
    auto scope = this;

    do
    {
        auto value = scope->properties.find(Name);
        if (value != scope->properties.end())
            return value->second;

        scope = scope->prototype.get();
    } while (scope != nullptr && SearchParents);

    return Plang::Undefined;
}

const Plang::AnyRef& Plang::Construct::Get(const std::string& Name, bool SearchParents) const
{
    auto scope = this;

    do
    {
        auto value = scope->properties.find(Name);
        if (value != scope->properties.end())
            return value->second;

        scope = scope->prototype.get();
    } while (scope != nullptr && SearchParents);

    return Plang::Undefined;
}

bool Plang::Construct::Has(const std::string& Name, bool SearchParents) const
{
    auto scope = this;

    do
    {
        if (scope->properties.find(Name) != scope->properties.end())
            return true;

        scope = scope->prototype.get();
    } while (scope != nullptr && SearchParents);

    return false;
}

bool Plang::Construct::Remove(const std::string& Name)
{
    auto value = properties.find(Name);
    if (value != properties.end())
    {
        properties.erase(value);
        return true;
    }
    return false;
}

Plang::Construct& Plang::Construct::Merge(const Plang::Construct& Other, bool Overwrite)
{
    if (&Other != this)
    {
        for (auto& var : Other.properties)
        {
            if (Overwrite || properties.find(var.first) != properties.end())
                properties.insert(var);
        }
    }

    return *this;
}

std::ostream& operator<<(std::ostream& Stream, const Plang::Construct& Construct)
{
    Stream << TypeToString(Construct.Type()) << "{" << Construct.ToString() << "}";
    return Stream;
}

std::string Plang::Construct::ToString() const
{
	std::string out;
	out += "{ ";
	if (properties.size() > 0)
	{
		auto last = --properties.end();
		for (auto i = properties.begin(); i != properties.end(); i++)
		{
			out += i->first + ": " + (i->second == nullptr ? "undefined" : i->second->ToString());
			if (i != last)
				out += ", ";
		}
	}
	out += " } <- " + (prototype == nullptr ? "undefined" : std::to_string((size_t)prototype.get()));
	return out;
}

Plang::Signature::Signature(const ::Array<Argument> Arguments)
    : arguments(Arguments), nSingles(0)
{
    for (size_t i = 0; i < arguments.Length(); i++)
    {
        if (arguments[i].type != ArgumentType::List)
            nSingles++;
    }
    nLists = arguments.Length() - nSingles;
}

Plang::Signature::Signature(const std::string& ArgsString)
{
    std::istringstream iss (ArgsString);
	Lexer lex ("@", iss);
    Parser parse;
    parse.Parse(lex.tokens, true);

    throw "todo";
}

Plang::Construct Plang::Signature::Parse(const Plang::Tuple& Arguments)
{
    Construct scope;

	auto nListArgs = (Arguments.Length() > nSingles ? Arguments.Length() - nSingles : 0); //the number of arguments passed in that will go into tuples
    auto nArgsPerList = nLists > 0 ? (1 + ((nListArgs - 1) / nLists)) : 0;

    for (size_t i = 0, j = 0; i < arguments.Length(); i++)
    {
		if (arguments[i].type == ArgumentType::List)
		{
			size_t nArgs = std::min(nArgsPerList, Arguments.value.Length() - j);
			if (nArgs > 0)
			{
				auto list(std::make_shared<List>(Arguments.value.Slice(j, nArgs)));
				scope.Set(arguments[i].name, list);
				j += nArgs;
			}
			else
			{
				scope.Set(arguments[i].name, std::make_shared<List>());
			}
		}
		else if (j < Arguments.Length())
			scope.Set(arguments[i].name, Arguments[j++]);
		else
			scope.Set(arguments[i].name, Undefined);
    }

	//todo: named properties

    return scope;
}

Plang::AnyRef Plang::Function::Call(const Plang::Tuple& Arguments, const Plang::AnyRef& LexScope)
{
    auto scope = signature.Parse(Arguments);
	scope.prototype = (LexScope == Undefined ? context : LexScope);

    return function(scope);
}

struct ScriptFrame
{
    Plang::Instruction& instruction; //must verify that tree does not change during execution
    size_t index; //index of current instruction
};

Plang::AnyRef Plang::Script::Evaluate(const Plang::Tuple& arguments, const AnyRef& lexScope)
{
    std::vector<AnyRef> registers; //holds intermediate values for functions
    std::stack<AnyRef> dot; //object scopes


    auto localScope(signature.Parse(arguments));
    localScope.prototype = (lexScope == Undefined ? context : lexScope);
    AnyRef pScope = std::make_shared<Construct>(localScope);

    std::stack<ScriptFrame> stack;
    stack.push({ instructions, 0 });

    while (!stack.empty())
    {
        auto& top = stack.top();
        auto children = std::get_if<TList>(&top.instruction.value);
        if (children != nullptr && top.index < children->size())
        {
            stack.push({ children->at(top.index), 0 });
            ++top.index;
            continue;
        }

        switch (top.instruction.type)
        {
        case InstructionType::Int:
            registers.push_back(std::make_shared<Plang::Int>(top.instruction.value));
            break;
        case InstructionType::Float:
            registers.push_back(std::make_shared<Plang::Float>(top.instruction.value));
            break;
        case InstructionType::String:
            registers.push_back(std::make_shared<Plang::String>(top.instruction.value));
            break;
        case InstructionType::Identifier:
            registers.push_back(localScope.Get(std::get<std::string>(top.instruction.value)));
            break;

        case InstructionType::Accessor:
            break;

        case InstructionType::Tuple:
        {
            auto count(top.instruction.As<Instructions::List>().Count());
            auto tup(std::make_shared<Tuple>(count));
            std::move(registers.end() - count, registers.end(), tup->value.Data()); //todo: revisit
            registers.resize(registers.size() - count);
            registers.push_back(tup);
            break;
        }
        case InstructionType::List:
        {
            auto count(top.instruction.As<Instructions::List>().Count());
            auto list(std::make_shared<List>());
            list->value.assign(registers.end() - count, registers.end()); //todo: revisit
            registers.resize(registers.size() - count);
            registers.push_back(list);
            break;
        }
        }

        stack.pop();
    }

    //return value of statement
    if (registers.size() > 1)
        throw "???";

    return registers.empty() ? Undefined : registers[0]; //todo: handle returning self in bound methods (?)
}