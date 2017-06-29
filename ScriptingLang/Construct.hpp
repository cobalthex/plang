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
		Bool,
		Int,
		Float,
		String,
		Tuple,
		Array,
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
		Construct(const AnyRef& Prototype = nullptr) : prototype(Prototype) { }
		Construct(const ::Array<std::pair<std::string, AnyRef>>& Defaults) : properties(&Defaults[0], &Defaults[0] + Defaults.Length()) { }
		virtual ~Construct() = default;

		virtual inline ConstructType Type() const { return ConstructType::Construct; }
		virtual std::string ToString() const; //basic value output. Constructs may have custom as operator which has a string conversion

		AnyRef&       Set(const std::string& Name, const AnyRef& Ref, bool SearchParents = false); //If SearchParents is true, property is set where it is defined or in this scope if not found
		AnyRef&       Get(const std::string& Name, bool SearchParents = true);
		const AnyRef& Get(const std::string& Name, bool SearchParents = true) const;
		bool          Has(const std::string& Name, bool SearchParents = true) const;
		bool          Remove(const std::string& Name);
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

		AnyRef prototype; //parent construct. properties inherit

	protected:
		PropMap properties; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)
	};

	//Null and Undefined are special references

	class Int : public Construct
	{
	public:
		using ValueType = TInt;

		Int() : value(0) { }
		Int(const ValueType& value) : value(value) { }
        Int(const Instruction::ValueType& value)
            : value(std::get<TInt>(value)) { }

		inline ConstructType Type() const override { return ConstructType::Int; }
		inline std::string ToString() const override { return std::to_string(value); }
		inline std::string ToString(int Radix) const { return std::to_string(value); } //todo

		ValueType value;
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

    //stores arguments (revisit)
	class Tuple : public Construct
	{
	public:
		using KvPair = std::pair<std::string, AnyRef>;

		Tuple() = default;
		Tuple(const ::Array<AnyRef>& Indices) : value(Indices) { }
		Tuple(const ::Array<AnyRef>& Indices, const ::Array<KvPair>& Keys) : value(Indices), Construct(Keys) { }
		Tuple(std::initializer_list<AnyRef> Init) : value(Init) { }

		inline ConstructType Type() const override { return ConstructType::Tuple; }
		inline std::string ToString() const override
		{
			std::string s = "(";
			for (size_t i = 0; i < value.Length(); i++)
			{
				s += value[i]->ToString();
				if (i < value.Length() - 1)
					s += ",";
			}
			s += ")";
			return s;
		}

		inline AnyRef& operator[](size_t Index) { return value[Index]; }
		inline const AnyRef& operator[](size_t Index) const { return value[Index]; }
		inline const size_t Length() const { return value.Length(); }

		::Array<AnyRef> value;
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
		std::string name;
		ArgumentType type;
	};

	class Signature
	{
	public:
		Signature() : arguments(), nSingles(0), nLists(0) { }
		Signature(const ::Array<Argument> Arguments);
		Signature(std::initializer_list<Argument> Arguments) : Signature(::Array<Argument>(Arguments)) { }
		Signature(const std::string& ArgsString);

		//Parse an arguments tuple using this signature. Named arguments overwrite positional arguments
		Construct Parse(const Tuple& Arguments);

	private:
		::Array<Argument> arguments;
		size_t nSingles; //the number of non-list arguments in the signature
		size_t nLists; //the number of list arguments in the signature
	};

	//A native function
	class Function : public Construct
	{
	public:
		using TFunction = std::function<AnyRef(Construct& Scope)>;

		inline Function(const Signature& Signature, const TFunction& Function, const Plang::AnyRef& Context = Undefined)
			: signature(Signature), function(Function), context(Context) { }
		inline Function(const TFunction& Function, const Plang::AnyRef& Context = Undefined)
			: function(Function), context(Context) { }

		inline ConstructType Type() const override { return ConstructType::Function; }
		inline std::string ToString() const override { return "[[ Native function ]]"; } //todo: base 16

		AnyRef Call(const Tuple& Arguments, const AnyRef& LexScope = Undefined);
		inline AnyRef Call(const AnyRef& LexScope = Undefined)
		{
			Tuple args;
			return Call(args, LexScope);
		}

		Signature signature;
		TFunction function;
		AnyRef context; //The lexical scope that this script can see. Typically the parent scope
	};

	//A parsed script. All scripts behave like functions, having arguments and a return value
	class Script : public Construct
	{
	public:
		inline Script(const Signature& signature, const Instruction& instructions, const Plang::AnyRef& context = Undefined)
			: signature(signature), instructions(instructions), context(context) { }
		inline Script(const Instruction& instructions, const Plang::AnyRef& context = Undefined)
			: instructions(instructions), context(context) { }

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
		AnyRef context; //The lexical scope that this script can see. Typically the parent scope
	};

    std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);
};