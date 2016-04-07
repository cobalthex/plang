#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Reference.hpp"

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
		virtual operator std::string() const;

	protected:
		Scope* scope; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)
	};

	class Bool : public Construct
	{
	public:
		Bool(bool Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Bool; }
		virtual inline operator std::string() const { return std::to_string(value); }

	protected:
		bool value;
	};

	class Int : public Construct
	{
	public:
		Int(IntT Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Int; }
		virtual inline operator std::string() const { return std::to_string(value); }

	protected:
		IntT value;
	};

	class String : public Construct
	{
	public:
		String(StringT Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::String; }
		virtual inline operator std::string() const { return value; }

	protected:
		StringT value;
	};

	class Array : public Construct
	{
	public:
		Array(size_t Length) : indices(Length) { }

		inline ValueType Type() const { return ValueType::String; }
		virtual inline operator std::string() const { return "[ Array (" + std::to_string(indices.size()) + ") ]"; }

	protected:
		std::vector<Reference> indices;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);