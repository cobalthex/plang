#pragma once

#include "pch.hpp"
#include "Array.hpp"
#include "Instruction.hpp"

namespace Plang
{
    class Construct;
    template <class T>
    using Reference = std::shared_ptr<T>;
    using AnyRef = Reference<Construct>;
    extern AnyRef Undefined;

	enum class ConstructType
	{
		Invalid,
		Construct,
		Int,
		Float,
		String,
		List,
		Function, //native function
		Script,
	};

	std::string TypeToString(const ConstructType& Type);

	//The basic object type. Everything is a construct from literals to lexical blocks to functions to scopes
	class Construct
	{
		using PropMap = std::map<std::string, AnyRef>;

	public:
        Construct(const AnyRef& prototype = nullptr) : prototype(prototype) { }
        Construct(const ::Array<std::pair<std::string, AnyRef>>& defaults) : properties(&defaults[0], &defaults[0] + defaults.Length()) { }
		virtual ~Construct() = default;

		virtual inline ConstructType Type() const { return ConstructType::Construct; }
		virtual std::string ToString() const; //basic value output. Constructs may have custom as operator which has a string conversion

		AnyRef&       Set(const std::string& name, const AnyRef& ref, bool searchInParents = false); //If searchInParents is true, property is set where it is defined or in this scope if not found
		AnyRef&       Get(const std::string& name, bool searchInParents = true);
		const AnyRef& Get(const std::string& name, bool searchInParents = true) const;
		bool          Has(const std::string& name, bool searchInParents = true) const;
		bool          Remove(const std::string& name);
		inline size_t Count() const { return properties.size(); } //The number of defined properties in this construct

		//iterators (For public properties)
		inline PropMap::iterator begin() { return properties.begin(); }
		inline PropMap::iterator end() { return properties.end(); }
		inline PropMap::const_iterator begin() const { return properties.begin(); }
		inline PropMap::const_iterator end() const { return properties.end(); }
		inline PropMap::const_iterator cbegin() const { return properties.cbegin(); }
		inline PropMap::const_iterator cend() const { return properties.cend(); }

		//Merge another construct's properties into this one (does not modify prototype or copy inherited properties). Returns a reference to this construct
		Construct& Merge(const Construct& Other, bool Overwrite = true);

        AnyRef prototype;

	protected:
		PropMap properties; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)
	};

	//Null and Undefined are special references

	class Int : public Construct
	{
	public:
		using ValueType = TInt;

		Int() : value(0) { Merge(Prototype); }
		Int(const ValueType& value) : value(value) { Merge(Prototype); }
        Int(const Instruction::ValueType& value)
            : value(std::get<TInt>(value)) { Merge(Prototype); }

		inline ConstructType Type() const override { return ConstructType::Int; }
		inline std::string ToString() const override { return std::to_string(value); }
		inline std::string ToString(int Radix) const { return std::to_string(value); } //todo

		ValueType value;

        static Construct Prototype;
	};

	class Float : public Construct
	{
	public:
		using ValueType = TFloat;

        Float() : value(0.0f) { }
		Float(const ValueType& value) : value(value) { }
        Float(const Instruction::ValueType& value)
            : value(std::get<TFloat>(value)) { }

		inline ConstructType Type() const override { return ConstructType::Float; }
		inline std::string ToString() const override { return std::to_string(value); }
		inline std::string ToString(int Radix) const { return std::to_string(value); }

		ValueType value;
	};

	class String : public Construct
	{
	public:
		using ValueType = std::string;

		String() : value("") { }
		String(const ValueType& value) : value(value) { }
		String(const ValueType::value_type* value) : value(value) { }
        String(const Instruction::ValueType& value)
            : value(std::get<std::string>(value)) { }

		inline ConstructType Type() const override { return ConstructType::String; }
		inline std::string ToString() const override { return "'" + value + "'"; }

		ValueType value;
	};

    struct Tuple
    {
        using Iterator = std::vector<AnyRef>::const_iterator;
        Tuple() { }
        Tuple(const Iterator& begin, const Iterator& end) : begin(begin), end(end) { }

        Iterator begin, end;

        size_t Length() const { return std::distance(begin, end); }

        Tuple Slice(size_t start, size_t count) const
        {
            count = std::min(Length(), count) - start;
            return Tuple(begin + start, begin + start + count);
        }

        const AnyRef& operator[](size_t index) const
        {
            return *(begin + index);
        }
    };

    /*
	class Array : public Construct
	{
	public:
		Array() = default;
		Array(const ::Array<AnyRef>& value) : value(value) { }
		Array(::Array<AnyRef>&& value) : value(value) { }

		inline ConstructType Type() const override { return ConstructType::Array; }
		inline std::string ToString() const override
		{
			std::string s = "[|";
			for (size_t i = 0; i < value.Length(); i++)
			{
				s += value[i]->ToString();
				if (i < value.Length() - 1)
					s += ",";
			}
			s += "|]";
			return s;
		}

		inline AnyRef& operator[](size_t Index) { return value[Index]; }
		inline const AnyRef& operator[](size_t Index) const { return value[Index]; }
		inline const size_t Length() const { return value.Length(); }

		::Array<AnyRef> value;
	};
    */

	class List : public Construct
	{
	public:
		List() = default;
		List(const std::vector<AnyRef>& values) : value(values) { }
        List(const ::Array<AnyRef>& array) : value(array.Data(), array.Data() + array.Length()) { }
        List(const Tuple& tuple) : value(tuple.begin, tuple.end) { }

		inline ConstructType Type() const override { return ConstructType::List; }
		inline std::string ToString() const override
		{
			std::string s = "[";
			for (size_t i = 0; i < value.size(); i++)
			{
				s += value[i]->ToString();
				if (i < value.size() - 1)
					s += ",";
			}
			s += "]";
			return s;
		}

		inline AnyRef& Add(const AnyRef& Ref) { value.push_back(Ref); return value.back(); }
		inline AnyRef Remove(const AnyRef& Ref) { AnyRef rval = value.back(); value.pop_back(); return rval; }

		inline AnyRef& operator[](size_t Index) { return value[Index]; }
		inline const AnyRef& operator[](size_t Index) const { return value[Index]; }
		inline const size_t Length() const { return value.size(); }

		std::vector<AnyRef> value;
	};

	enum class ArgumentType
	{
		Single,
		List,
	};

	struct Argument
	{
        Argument() = default;
        Argument(std::string name, ArgumentType type = ArgumentType::Single)
            : name(std::move(name)), type(type) { }

		std::string name;
		ArgumentType type;
	};

	class Signature
	{
	public:
		Signature() = default;
		Signature(const ::Array<Argument> arguments);
		Signature(std::initializer_list<Argument> arguments) : Signature(::Array<Argument>(arguments)) { }
		Signature(const std::string& argsString);

		//Parse an arguments tuple using this signature. Named arguments overwrite positional arguments
		Construct Parse(const Tuple& Arguments);

	private:
		::Array<Argument> arguments;
		size_t nSingles = 0; //the number of non-list arguments in the signature
		size_t nLists = 0; //the number of list arguments in the signature
	};

	//A native function
	class Function : public Construct
	{
	public:
		using TFunction = std::function<AnyRef(Construct& Scope)>;

		inline Function(const Signature& signature, const TFunction& callable)
			: signature(signature), function(callable) { }
		inline Function(const TFunction& callable)
			: function(callable) { }

		inline ConstructType Type() const override { return ConstructType::Function; }
		inline std::string ToString() const override { return "[[ Native function ]]"; } //todo: base 16

		AnyRef Call(const Tuple& Arguments, const AnyRef& LexScope = Undefined); //raw construct?
		inline AnyRef Call(const AnyRef& LexScope = Undefined)
		{
			Tuple args;
			return Call(args, LexScope);
		}

		Signature signature;
		TFunction function;
        AnyRef boundScope; //todo
	};

	//A parsed script. All scripts behave like functions, having arguments and a return value
	class Script : public Construct
	{
	public:
		Script(const Signature& signature, const Instruction& rootInstruction)
			: signature(signature), instructions(rootInstruction) { }
		Script(const Instruction& rootInstruction)
			: instructions(rootInstruction)  { }

		inline ConstructType Type() const override { return ConstructType::Script; }
		inline std::string ToString() const override { return "[[ Script (" + instructions.TypeName() + ") ]]"; } //todo: print signature

		AnyRef Evaluate(const Tuple& arguments, const AnyRef& lexScope = Undefined);
        //todo: limited evaluation (N instructions, returns task/etc)
		inline AnyRef Evaluate(const AnyRef& lexScope = Undefined)
		{
			Tuple args;
			return Evaluate(args, lexScope);
		}

		Signature signature;
		Instruction instructions;
        AnyRef boundScope; //todo

    protected:
	};

    std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);
    std::ostream& operator << (std::ostream& Stream, const Plang::AnyRef& Ref);
};