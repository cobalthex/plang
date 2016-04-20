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
		virtual ~Construct() = default;

		virtual inline ConstructType Type() const { return ConstructType::Construct; }
		virtual inline std::string ToString() const { return "[Object]"; } //basic string output (inherited classes may have multiple versions)

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

		inline ConstructType Type() const override { return ConstructType::String; }
		inline std::string ToString() const override { return "[ Array (" + std::to_string(value.Length()) + ") ]"; }

		::Array<AnyRef> value;
	};

	//A script
	class Script : public Construct
	{
	public:
		Script() : Construct() { }

		inline ConstructType Type() const override { return ConstructType::Script; }
		inline std::string ToString() const override { return "[[ Script ]]"; }

		SyntaxTreeNode* node;

		AnyRef Evaluate(Scope* LexScope);
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
		::Array<Argument> arguments;
		Scope Parse(const Tuple& Arguments);
	};

	class Function : public Construct
	{
	public:
		inline std::string ToString() const override { return "[[ Native function ]]"; }

		Plang::AnyRef Call();

		Signature signature;
	};

	class ScriptFunction : public Construct
	{
	public:
		ScriptFunction() = default;

		inline ConstructType Type() const override { return ConstructType::ScriptFunction; }
		inline std::string ToString() const override { return "[[ Script function ]]"; }

		Script script;
		Signature signature;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);
