#include "pch.hpp"
#include "Construct.hpp"

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
Plang::AnyRef& Plang::Construct::Set(const Plang::StringT& Name, const Plang::AnyRef& Ref, bool SearchParents)
{
    auto scope = this;

	if (prototype != nullptr)
	{
		do
		{
			if (scope->properties.find(Name) != scope->properties.end())
				break;

			scope = scope->prototype.Get();
		} while (scope != nullptr);
	}

    return (scope->properties[Name] = Ref);
}

Plang::AnyRef& Plang::Construct::Get(const Plang::StringT& Name, bool SearchParents)
{
    auto scope = this;

    do
    {
        auto value = scope->properties.find(Name);
        if (value != scope->properties.end())
            return value->second;

        scope = scope->prototype.Get();
    } while (scope != nullptr && SearchParents);

    return Plang::Undefined;
}

const Plang::AnyRef& Plang::Construct::Get(const Plang::StringT& Name, bool SearchParents) const
{
    auto scope = this;

    do
    {
        auto value = scope->properties.find(Name);
        if (value != scope->properties.end())
            return value->second;

        scope = scope->prototype.Get();
    } while (scope != nullptr && SearchParents);

    return Plang::Undefined;
}

bool Plang::Construct::Has(const Plang::StringT& Name, bool SearchParents) const
{
    auto scope = this;

    do
    {
        if (scope->properties.find(Name) != scope->properties.end())
            return true;

        scope = scope->prototype.Get();
    } while (scope != nullptr && SearchParents);

    return false;
}

bool Plang::Construct::Remove(const Plang::StringT& Name)
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
    //Stream << TypeToString(Construct.Type()) << "(" << Construct.ToString() << ")";
    std::cout << "Prototype: ";
    if (Construct.prototype == nullptr)
		std::cout << "null\n";
    else
		std::cout << std::hex << std::showbase << Construct.prototype.Get() << "\n";

    std::cout << "properties:\n";
    for (auto& prop : Construct.properties)
		std::cout << "  " << prop.first << ": " << (prop.second == nullptr ? "Undefined" : prop.second->ToString()) << "\n";
    std::cout << "\n";
    return Stream;
}

Plang::Signature::Signature(const ::Array<Argument> Arguments)
    : arguments(Arguments), nSingles(0)
{
    for (size_t i = 0; i < arguments.Length(); i++)
    {
        if (arguments[i].type == ArgumentType::Tuple)
            nSingles++;
    }
    nTuples = arguments.Length() - nSingles;
}

Plang::Construct Plang::Signature::Parse(const Plang::Tuple& Arguments)
{
    Construct scope;

    auto nTupleArgs = Arguments.Length() - nSingles;
    auto nArgsPerTuple = nTuples > 0 ? (1 + ((nTupleArgs - 1) / nTuples)) : 0;

    for (size_t i = 0; i < std::min(arguments.Length(), Arguments.Length());)
    {
        auto& prop = Arguments.Get(arguments[i].name);
        if (prop != Undefined)
            scope.Set(arguments[i].name, prop);
        else if (arguments[i].type == ArgumentType::Tuple)
        {
            size_t nArgs = std::min(nArgsPerTuple, Arguments.value.Length() - i);
            if (nArgs > 0)
            {
                Reference<Tuple> tup (Arguments.value.Slice(i, nArgs));
                scope.Set(arguments[i].name, tup);
                i += nArgs;
            }
        }
        else
        {
            scope.Set(arguments[i].name, Arguments[i]);
            i++;
        }
    }

    return scope;
}

Plang::AnyRef Plang::Function::Call(const Plang::Tuple& Arguments, const Plang::AnyRef& LexScope)
{
    auto scope = signature.Parse(Arguments);
    scope.prototype = LexScope;

    return function(scope);
}

struct Frame
{
    Plang::SyntaxTreeNode* node; //the node to evaluate
    size_t index; //index of current instruction
};

inline bool IsSymbol(Plang::SyntaxTreeNode* Node, size_t Index)
{
    Node = Node->parent;
    return (
        Node != nullptr
        && Index == 0
        && Node->instruction.type == Plang::InstructionType::Call
        && Node->children.size() > 1
        && (Node->instruction.value == "=" || Node->instruction.value == ":")
    );
}

Plang::AnyRef Plang::Script::Evaluate(const Plang::Tuple& Arguments, const AnyRef& LexScope)
{
    if (node == nullptr)
        return Undefined;

    std::vector<AnyRef> registers; //holds intermediate values for functions

    auto scope = signature.Parse(Arguments);
    scope.prototype = LexScope;

    AnyRef pScope = scope;
	Construct* dot; //the construct for which dot properties should be set

    std::stack<Frame> stack;
    stack.push({ node, 0 });
    while (!stack.empty())
    {
        auto top = stack.top();
        stack.pop();

		if (top.node->instruction.type == InstructionType::Block)
			registers.push_back(Construct());
		else if (top.node->instruction.type == InstructionType::Expression)
		{
			//first child is arguments
			//second child is script
			continue;
		}

        bool nest = false;
        for (size_t i = top.index; i < top.node->children.size(); i++)
        {
            auto& child = top.node->children[i];

            if (child.instruction.type == InstructionType::Accessor)
                ; //todo
            if (child.instruction.type == InstructionType::Integer)
                registers.push_back(Reference<Int>(child.instruction.value.get<Int::ValueType>()));
            else if (child.instruction.type == InstructionType::Float)
                registers.push_back(Reference<Float>(child.instruction.value.get<Float::ValueType>()));
            else if (child.instruction.type == InstructionType::String)
                registers.push_back(Reference<String>(child.instruction.value.get<String::ValueType>()));
            else if (child.instruction.type == InstructionType::Identifier)
                registers.push_back(scope.Get(child.instruction.value.get<StringT>()));
            else if (child.children.size() > 0)
            {
                stack.push({ &child, 0 });
                nest = true;
                i++;
                break;
            }
        }

        if (!nest)
        {
			if (top.node->instruction.type == InstructionType::Block)
				;
				
            auto& inst = top.node->instruction;
            if (inst.type == InstructionType::Tuple)
            {
                auto len = top.node->children.size();
                auto start = registers.size() - len;
                Plang::Tuple tup(::Array<AnyRef>(registers.data() + start, len));
                registers.erase(registers.begin() + start, registers.begin() + start + len);
                registers.push_back(tup);
            }
            else if (inst.type == InstructionType::Array)
            {
                auto len = top.node->children.size();
                auto start = registers.size() - len;
                Plang::Array tup(::Array<AnyRef>(registers.data() + start, len));
                registers.erase(registers.begin() + start, registers.begin() + start + len);
                registers.push_back(tup);
            }
            else if (inst.type == InstructionType::Call)
            {
                auto& fn = scope.Get(inst.value.get<StringT>());

                auto len = top.node->children.size();
                auto start = registers.size() - len;
                Tuple tup (::Array<AnyRef>(registers.data() + start, len));
                registers.erase(registers.begin() + start, registers.begin() + start + len);

                if (fn == Undefined)
                    throw inst.value.get<StringT>() + " is undefined and not callable";
                else if (fn->Type() == ConstructType::Script)
                    registers.push_back(Reference<Script>(fn)->Evaluate(tup, pScope));
                else if (fn->Type() == ConstructType::Function)
                    registers.push_back(Reference<Function>(fn)->Call(tup, pScope));
                else
                    throw "not callable";
            }
            //any other instructions can be ignored (todo: verify)
        }
    }

    return Undefined;
}
