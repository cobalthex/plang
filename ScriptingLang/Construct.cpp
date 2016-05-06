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
    Stream << TypeToString(Construct.Type()) << "(" << Construct.ToString() << ")";
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
	out += " } <- " + (prototype == nullptr ? "undefined" : std::to_string((size_t)prototype.Get()));
	return out;
}

Plang::Signature::Signature(const ::Array<Argument> Arguments)
    : arguments(Arguments), nSingles(0)
{
    for (size_t i = 0; i < arguments.Length(); i++)
    {
        if (arguments[i].type != ArgumentType::Tuple)
            nSingles++;
    }
    nTuples = arguments.Length() - nSingles;
}

Plang::Construct Plang::Signature::Parse(const Plang::Tuple& Arguments)
{
    Construct scope;

	auto nTupleArgs = (Arguments.Length() > nSingles ? Arguments.Length() - nSingles : 0); //the number of arguments passed in that will go into tuples
    auto nArgsPerTuple = nTuples > 0 ? (1 + ((nTupleArgs - 1) / nTuples)) : 0;

    for (size_t i = 0, j = 0; i < arguments.Length(); i++)
    {
		if (arguments[i].type == ArgumentType::Tuple)
		{
			size_t nArgs = std::min(nArgsPerTuple, Arguments.value.Length() - j);
			if (nArgs > 0)
			{
				Reference<Tuple> tup(Arguments.value.Slice(j, nArgs));
				scope.Set(arguments[i].name, tup);
				j += nArgs;
			}
			else
			{
				scope.Set(arguments[i].name, Reference<Tuple>::New());
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

struct Frame
{
    Plang::SyntaxTreeNode* node; //the node to evaluate
    size_t index; //index of current instruction
};

inline bool IsSymbol(Plang::SyntaxTreeNode* Node, size_t Index)
{
    return (Index == 0
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
    scope.prototype = (LexScope == Undefined ? context : LexScope);

    AnyRef pScope = scope;

    std::stack<Frame> stack;
    stack.push({ node, 0 });
    while (!stack.empty())
    {
        auto top = stack.top();
        stack.pop();

		if (top.node->instruction.type == InstructionType::Block) //todo: verify necessary
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
			{
				if (IsSymbol(top.node, i))
				{
					//node is a symbol and therefore is an assignment

					registers.push_back(scope.Get(child.instruction.value.get<StringT>()));
				}
				else
					registers.push_back(scope.Get(child.instruction.value.get<StringT>()));
			}
			//continue recursing
			else if (child.children.size() > 0)
			{
				stack.push({ &child, 0 });
				nest = true;
				i++;
				break;
			}
			//empty containers
			else if (child.instruction.type == InstructionType::Block)
				registers.push_back(Reference<Construct>::New());
			else if (child.instruction.type == InstructionType::Array)
				registers.push_back(Reference<Array>::New()); //todo: this should be an error (no empty arrays)
			else if (child.instruction.type == InstructionType::Tuple)
				registers.push_back(Reference<Tuple>::New());
			else if (child.instruction.type == InstructionType::List)
				registers.push_back(Reference<List>::New());
			else if (child.instruction.type == InstructionType::Call)
				stack.push({ &child, 0 });
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
				registers.erase(registers.end() - len, registers.end());
				registers.push_back(tup);
			}
			else if (inst.type == InstructionType::Array)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				Plang::Array arr(::Array<AnyRef>(registers.data() + start, len));
				registers.erase(registers.end() - len, registers.end());
				registers.push_back(arr);
			}
			else if (inst.type == InstructionType::List)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				Plang::List list(std::vector<AnyRef>(registers.end() - len, registers.end()));
				registers.erase(registers.end() - len, registers.end());
				registers.push_back(list);
			}
			else if (inst.type == InstructionType::Call)
			{
				auto& fn = scope.Get(inst.value.get<StringT>());

				auto len = top.node->children.size();
				auto start = registers.size() - len;
				Tuple tup(::Array<AnyRef>(registers.data() + start, len));
				registers.erase(registers.end() - len, registers.end());

				//todo: nested tuples don't work

				if (fn == Undefined)
					//throw inst.value.get<StringT>() + " is undefined and not callable";
					std::cout << inst.value.get<StringT>() + " is undefined and not callable\n";
				else if (fn->Type() == ConstructType::Script)
					registers.push_back(Reference<Script>(fn)->Evaluate(tup));
				else if (fn->Type() == ConstructType::Function)
					registers.push_back(Reference<Function>(fn)->Call(tup));
				else
					throw "not callable";
			}
			//todo: expressions, etc

			//any other instructions can be ignored (todo: verify)
        }
    }

	//return value of statement
	if (registers.size() > 0)
		std::cout << ">> " << registers.back() << "\n";

    return Undefined;
}
