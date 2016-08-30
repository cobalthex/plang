#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Array.hpp"
#include "Reference.hpp"
#include "SyntaxTree.hpp"

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);

namespace Plang
{
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
		using PropT = std::map<StringT, AnyRef>;

	public:
		Construct(const AnyRef& Prototype = nullptr) : prototype(Prototype) { }
		Construct(const ::Array<std::pair<StringT, AnyRef>>& Defaults) : properties(&Defaults[0], &Defaults[0] + Defaults.Length()) { }
		virtual ~Construct() = default;

		virtual inline ConstructType Type() const { return ConstructType::Construct; }
		virtual std::string ToString() const; //basic value output. Constructs may have custom as operator which has a string conversion

		AnyRef&       Set(const StringT& Name, const AnyRef& Ref, bool SearchParents = false); //If SearchParents is true, property is set where it is defined or in this scope if not found
		AnyRef&       Get(const StringT& Name, bool SearchParents = true);
		const AnyRef& Get(const StringT& Name, bool SearchParents = true) const;
		bool          Has(const StringT& Name, bool SearchParents = true) const;
		bool          Remove(const StringT& Name);
		inline size_t Count() const { return properties.size(); } //The number of defined properties in this construct

		//iterators (For public properties)
		inline PropT::iterator begin() { return properties.begin(); }
		inline PropT::iterator end() { return properties.end(); }
		inline PropT::const_iterator begin() const { return properties.begin(); }
		inline PropT::const_iterator end() const { return properties.end(); }
		inline PropT::const_iterator cbegin() const { return properties.cbegin(); }
		inline PropT::const_iterator cend() const { return properties.cend(); }

		//Merge another construct's properties into this one (does not modify prototype or copy inherited properties). Returns a reference to this construct
		Construct& Merge(const Construct& Other, bool Overwrite = true);

		AnyRef prototype; //parent construct. properties inherit

	protected:
		PropT properties; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)

		friend std::ostream& (::operator <<)(std::ostream&, const Construct&);
	};

	//Null and Undefined are special references

	class Bool : public Construct
	{
	public:
		using ValueType = bool;

		Bool() : value(false) { }
		Bool(const ValueType& Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::Bool; }
		inline std::string ToString() const override { return std::to_string(value); }

		ValueType value;
	};

	class Int : public Construct
	{
	public:
		using ValueType = IntT;

		Int() : value(0) { }
		Int(const ValueType& Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::Int; }
		inline std::string ToString() const override { return std::to_string(value); }
		inline std::string ToString(int Radix) const { return std::to_string(value); } //todo

		ValueType value;
	};

	class Float : public Construct
	{
	public:
		using ValueType = FloatT;

		Float() : value(0.0f) { }
		Float(const ValueType& Value) : value(Value) { }

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
		String(const ValueType& Value) : value(Value) { }
		String(const ValueType::value_type* Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::String; }
		inline std::string ToString() const override { return "'" + value + "'"; }

		ValueType value;
	};

	//A fixed size, immutable data type
	//These are typically meant for storing
	//Supports both named properties (stored in construct props) and indices
	class Tuple : public Construct
	{
	public:
		using KvPair = std::pair<StringT, AnyRef>;

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

	class Array : public Construct
	{
	public:
		Array() = default;
		Array(const ::Array<AnyRef>& Value) : value(Value) { }
		Array(::Array<AnyRef>&& Value) : value(Value) { }

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

	class List : public Construct
	{
	public:
		List() = default;
		List(const std::vector<AnyRef>& Value) : value(Value) { }
		List(std::vector<AnyRef>&& Value) : value(Value) { }

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
		Tuple,
	};

	struct Argument
	{
		StringT name;
		ArgumentType type;
	};

	class Signature
	{
	public:
		Signature() : arguments(), nSingles(0), nTuples(0) { }
		Signature(const ::Array<Argument> Arguments);
		Signature(std::initializer_list<Argument> Arguments) : Signature(::Array<Argument>(Arguments)) { }
		Signature(const StringT& ArgsString);

		//Parse an arguments tuple using this signature. Named arguments overwrite positional arguments
		Construct Parse(const Tuple& Arguments);

	private:
		::Array<Argument> arguments;
		size_t nSingles; //the number of non-tuple arguments in the signature
		size_t nTuples; //the number of tuple arguments in the signature
	};

	//A native function
	class Function : public Construct
	{
	public:
		using FunctionT = std::function<AnyRef(Construct& Scope)>;

		inline Function(const Signature& Signature, const FunctionT& Function, const Plang::AnyRef& Context = Undefined)
			: signature(Signature), function(Function), context(Context) { }
		inline Function(const FunctionT& Function, const Plang::AnyRef& Context = Undefined)
			: function(Function), context(Context) { }

		inline ConstructType Type() const override { return ConstructType::Function; }
		inline std::string ToString() const override { return "[[ Native function ]]"; }

		AnyRef Call(const Tuple& Arguments, const AnyRef& LexScope = Undefined);
		inline AnyRef Call(const AnyRef& LexScope = Undefined)
		{
			Tuple args;
			return Call(args, LexScope);
		}

		Signature signature;
		FunctionT function;
		AnyRef context; //The lexical scope that this script can see. Typically the parent scope
	};

	//A script. All scripts behave like functions, having arguments and a return value
	class Script : public Construct
	{
	public:
		inline Script(const Signature& Signature, const SyntaxTreeNode& Node, const Plang::AnyRef& Context = Undefined)
			: signature(Signature), node(Node), context(Context) { }
		inline Script(const SyntaxTreeNode& Node, const Plang::AnyRef& Context = Undefined)
			: node(Node), context(Context) { }

		inline ConstructType Type() const override { return ConstructType::Script; }
		inline std::string ToString() const override { return "[[ Script ]]"; } //todo: print signature

		AnyRef Evaluate(const Tuple& Arguments, const AnyRef& LexScope = Undefined);
		inline AnyRef Evaluate(const AnyRef& LexScope = Undefined)
		{
			Tuple args;
			return Evaluate(args, LexScope);
		}

		Signature signature;
		SyntaxTreeNode node;
		AnyRef context; //The lexical scope that this script can see. Typically the parent scope
	};
};