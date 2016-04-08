#pragma once

#include "pch.hpp"
#include "types.hpp"
#include "Array.hpp"
#include "Reference.hpp"
#include "Scope.hpp"

namespace Plang
{
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
		virtual operator std::string() const; //basic debug output

		template <class T>
		inline T& As()
		{
			static_assert(std::is_convertible<Construct, T>::value, "T must be a subclass of Construct");
			//check types, if not same, cast
			return *(T*)this;
		}
		template <class T>
		inline const T& As() 
		{
			static_assert(std::is_convertible<Construct, T>::value, "T must be a subclass of Construct");
			return *(T*)this;
		}

		Scope properties; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)
	};

	class Bool : public Construct
	{
	public:
		Bool(bool Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Bool; }
		virtual inline operator std::string() const { return std::to_string(value); }

		bool value;
	};

	class Int : public Construct
	{
	public:
		Int(IntT Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Int; }
		virtual inline operator std::string() const { return std::to_string(value); }

		IntT value;
	};

	class String : public Construct
	{
	public:
		String(const StringT& Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::String; }
		virtual inline operator std::string() const { return "\"" + value + "\""; }

		const StringT value;
	};

	class Array : public Construct
	{
	public:
		Array(size_t Length) : indices(Length) { }
		~Array();

		inline ValueType Type() const { return ValueType::String; }
		virtual inline operator std::string() const { return "[ Array (" + std::to_string(indices.Length()) + ") ]"; }

		::Array<Reference> indices;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);