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

	while (scope->prototype != nullptr)
	{
		if (scope->properties.find(Name) != scope->properties.end())
			break;

		scope = scope->prototype.Get();
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

inline bool IsSymbol(Plang::SyntaxTreeNode* Node, size_t Index)
{
    return (Index == 0
        && Node->instruction.type == Plang::InstructionType::Call
        && Node->children.size() > 1
        && (Node->instruction.value == "=" || Node->instruction.value == ":")
    );
}

inline Plang::StringT GetNameString(const Plang::Instruction& Instruction)
{
	switch (Instruction.type)
	{
	case Plang::InstructionType::Integer:
		return std::to_string(Instruction.value.get<Plang::IntT>());
	case Plang::InstructionType::Float:
		return std::to_string(Instruction.value.get<Plang::FloatT>());
	default:
		return Instruction.value.get<Plang::StringT>();
	}
}

struct Frame
{
	Plang::SyntaxTreeNode* node; //the node to evaluate
	size_t index; //index of current instruction

	Plang::AnyRef* assignment; //assignment
};

Plang::AnyRef Plang::Script::Evaluate(const Plang::Tuple& Arguments, const AnyRef& LexScope)
{
    std::vector<AnyRef> registers; //holds intermediate values for functions
	std::stack<AnyRef> dot; //object scopes

	//scope here is the local scope
    auto scope = signature.Parse(Arguments);
    scope.prototype = (LexScope == Undefined ? context : LexScope);
    AnyRef pScope = scope;

    std::stack<Frame> stack;
    stack.push({ &node, 0 });
    while (!stack.empty())
    {
        auto& top = stack.top();

		if (top.index == 0 && top.node->instruction.type == InstructionType::Block)
		{
			dot.push(Reference<Construct>::New());
			registers.push_back(dot.top());
		}

		/*if (top.node->instruction.type == InstructionType::Expression)
		{
			//first child is arguments
			//second child is script
			continue;
		}*/

        bool nest = false;
		for (; top.index < top.node->children.size(); top.index++)
		{
			auto& child = top.node->children[top.index];

			//todo: handling setting non identifiers as properties

			if (child.instruction.type == InstructionType::Integer)
				registers.push_back(Reference<Int>(child.instruction.value.get<Int::ValueType>()));
			else if (child.instruction.type == InstructionType::Float)
				registers.push_back(Reference<Float>(child.instruction.value.get<Float::ValueType>()));
			else if (child.instruction.type == InstructionType::String)
				registers.push_back(Reference<String>(child.instruction.value.get<String::ValueType>()));
			else if (child.instruction.type == InstructionType::Accessor)
			{
				AnyRef* acc = &scope.Get(GetNameString(child.children[0].instruction));
				for (auto i = 1; i < child.children.size(); i++)
				{

				}
			}
			else if (child.instruction.type == InstructionType::Identifier)
			{
				if (IsSymbol(top.node, top.index))
				{
					//node is an assignment and therefore the first operator is an assignment

					auto name = child.instruction.value.get<StringT>();
					auto iname = top.node->instruction.value.get<StringT>();

					//store in existing location if found, otherwise create new in current scope
					if (iname == "=")
					{
						auto& assn = scope.Get(name);
						if (assn == Undefined)
							top.assignment = &scope.Set(name, AnyRef());
						else
							top.assignment = &assn;
					}
					else if (iname == ":")
					{
						if (dot.size() < 1)
							throw "Invalid assignment, must be in object/tuple"; //must cancel outer call
						else
							top.assignment = &dot.top()->Set(name, AnyRef());
					}
					//else what?
				}
				else
					registers.push_back(scope.Get(child.instruction.value.get<StringT>()));
			}
			//continue recursing
			else if (child.children.size() > 0)
			{
				stack.push({ &child, 0 });
				nest = true;
				top.index++;
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
			stack.pop();

			auto& inst = top.node->instruction;

			if (inst.type == InstructionType::Tuple)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				::Array<AnyRef> vals (registers.data() + start, len);
				registers.erase(registers.end() - len, registers.end());
				registers.push_back(Reference<Tuple>(vals));
			}
			else if (inst.type == InstructionType::Array)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				::Array<AnyRef> vals (registers.data() + start, len);
				registers.erase(registers.end() - len, registers.end());
				registers.push_back(Reference<Array>(vals));
			}
			else if (inst.type == InstructionType::List)
			{
				auto len = top.node->children.size();
				auto start = registers.size() - len;
				std::vector<AnyRef> vals (registers.end() - len, registers.end());
				registers.erase(registers.end() - len, registers.end());
				registers.push_back(Reference<List>(vals));
			}
			else if (inst.type == InstructionType::Call)
			{
				auto fnName = inst.value.get<StringT>();
				auto& fn = scope.Get(fnName);

				auto len = top.node->children.size();

				if (len > 0 && (fnName == "=" || fnName == ":"))
				{
					len--;
					*top.assignment = *(registers.end() - len);
					registers.erase(registers.end() - len, registers.end());
					registers.push_back(*top.assignment);
				}
				//todo: return
				else
				{
					auto start = registers.size() - len;
					Tuple tup (::Array<AnyRef>(registers.data() + start, len));
					registers.erase(registers.end() - len, registers.end());

					//todo: nested tuples don't work

					if (fn == Undefined)
						//throw fnName + " is undefined and not callable";
						std::cout << fnName + " is undefined and not callable\n";
					else if (fn->Type() == ConstructType::Script)
						registers.push_back(Reference<Script>(fn)->Evaluate(tup));
					else if (fn->Type() == ConstructType::Function)
						registers.push_back(Reference<Function>(fn)->Call(tup));
					else
						throw "not callable";
				}
			}
			else if (inst.type == InstructionType::Block)
			{
				//statements will store their return value as a register
				//without a return statement however, the block is the return value
				while (registers.back() != dot.top())
					registers.pop_back();
			}
			//todo: expressions, etc

			//any other instructions can be ignored (todo: verify)
        }
    }

	//return value of statement
	if (registers.size() > 0)
		std::cout << ">> " << registers.front() << "\n";

    return Undefined;
}
