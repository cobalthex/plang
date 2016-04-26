#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Array.hpp"
#include "Reference.hpp"
#include "Scope.hpp"
#include "SyntaxTree.hpp"

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
		Array,
		Tuple,
		Script,
		Function, //native function
		ScriptFunction,
	};

	std::string TypeToString(const Plang::ConstructType& Type);

	//The basic object type. Everything is a construct from literals to lexical blocks to functions
	class Construct
	{
	public:
		Construct() = default;
		Construct(const Scope& DefaultProps) : properties(DefaultProps) { }
		Construct(Scope&& DefaultProps) : properties(DefaultProps) { }
		virtual ~Construct() = default;

		virtual inline ConstructType Type() const { return ConstructType::Construct; }
		virtual inline std::string ToString() const { return "[[ Construct (" + std::to_string(Count()) + ") ]]"; } //basic string output (inherited classes may have multiple versions)

		//alises to property functions

		inline AnyRef& Set(const StringT& Name, const AnyRef& Ref) { return properties.Set(Name, Ref, false); }
		inline AnyRef& Get(const StringT& Name) { return properties.Get(Name, true); }
		inline const AnyRef& Get(const StringT& Name) const { return properties.Get(Name, true); }
		inline const size_t Count() const { return properties.Count(); }

		Scope properties; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)
	};

	//Null and Undefined are special references

	class Bool : public Construct
	{
	public:
		Bool() : value(false) { }
		Bool(const bool& Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::Bool; }
		inline std::string ToString() const override { return std::to_string(value); }

		bool value;
	};

	class Int : public Construct
	{
	public:
		Int() : value(0) { }
		Int(const IntT& Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::Int; }
		inline std::string ToString() const override { return std::to_string(value); }
		inline std::string ToString(int Radix) const { return std::to_string(value); }

		IntT value;
	};

	class Float : public Construct
	{
	public:
		Float() : value(0.0f) { }
		Float(const FloatT& Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::Float; }
		inline std::string ToString() const override { return std::to_string(value); }

		FloatT value;
	};

	class String : public Construct
	{
	public:
		String() : value("") { }
		String(const StringT& Value) : value(Value) { }
		String(const char* Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::String; }
		inline std::string ToString() const override { return "\"" + value + "\""; }

		StringT value;
	};

	class Array : public Construct
	{
	public:
		Array() = default;
		Array(const ::Array<AnyRef>& Value) : value(Value) { }
		Array(::Array<AnyRef>&& Value) : value(Value) { }

		inline ConstructType Type() const override { return ConstructType::Array; }
		inline std::string ToString() const override { return "[ Array (" + std::to_string(Length()) + ") ]"; }

		inline AnyRef& Set(size_t Index, const AnyRef& Ref) { return (value[Index] = Ref); }
		inline AnyRef& Get(size_t Index) { return value[Index]; }
		inline const AnyRef& Get(size_t Index) const { return value[Index]; }
		inline const size_t Length() const { return value.Length(); }

		::Array<AnyRef> value;
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
		inline std::string ToString() const override { return "[[ Tuple (" + std::to_string(Length()) + "," + std::to_string(Count()) + ") ]]"; }

		inline const AnyRef& Get(size_t Index) const { return value[Index]; }
		inline const size_t Length() const { return value.Length(); }

		::Array<AnyRef> value;
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

	struct Signature
	{
		::Array<Argument> signature;
		Scope Parse(const Tuple& Arguments); //Parse an arguments tuple using the signature. Named arguments overwrite positional arguments
	};

	//A native function
	class Function : public Construct
	{
	public:
		inline std::string ToString() const override { return "[[ Native function ]]"; }

		Plang::AnyRef Call(Scope* LexScope, const Tuple& Arguments);

		Signature signature;

		std::function<AnyRef(const Scope& Scope)> function;
	};

	//A script. All scripts behave like functions, having arguments and a return value
	class Script : public Construct
	{
	public:
		Script() : Construct() { }

		inline ConstructType Type() const override { return ConstructType::Script; }
		inline std::string ToString() const override { return "[[ Script ]]"; }

		AnyRef Evaluate(Scope* LexScope, const Tuple& Arguments);

		Signature signature;
		SyntaxTreeNode* node;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);
