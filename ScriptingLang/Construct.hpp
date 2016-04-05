#pragma once

#include "pch.hpp"
#include "types.hpp"

namespace Plang
{
	class Scope;

	enum class ValueType
	{
		None,
		Bool,
		Int,
		Float,
		String,
		Function, //native function
		Script,
		//Collections
		Array,
	};

	std::string TypeToString(const Plang::ValueType& Type);

	//The basic object. Everything is a construct from literals to lexical blocks to functions
	class Construct
	{
	public:
		Construct() { }
		~Construct() { }

		virtual inline ValueType Type() const { return ValueType::None; }

	protected:
		Scope* scope; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)
	};

	class Bool : public Construct
	{
	public:
		Bool(bool Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Bool; }

	protected:
		bool value;
	};

	class Int : public Construct
	{
	public:
		Int(IntT Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Int; }

	protected:
		IntT value;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);