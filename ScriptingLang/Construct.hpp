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

		//Reinterpret cast. Throws an error if types don't match
		template <class T>
		inline T& As()
		{
			static_assert(std::is_convertible<Construct, T>::value, "T must be a subclass of Construct");
			if (Type() != Typeof(T))
				throw
			return *(T*)this;
		}
		//returns a cast or a converted copy
		template <class T>
		inline const T& As() const
		{
			static_assert(std::is_convertible<Construct, T>::value, "T must be a subclass of Construct");
			return *(T*)this;
		}

		Scope properties; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)

		template <typename T> static constexpr inline ValueType Typeof() noexcept { return ValueType::None; }
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

	class Float : public Construct
	{
	public:
		Float(FloatT Value) : value(Value) { }

		inline ValueType Type() const { return ValueType::Float; }
		virtual inline operator std::string() const { return std::to_string(value); }

		FloatT value;
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

	template <> constexpr inline ValueType Construct::Typeof<Bool>() noexcept   { return ValueType::Bool; }
	template <> constexpr inline ValueType Construct::Typeof<Int>() noexcept    { return ValueType::Int; }
	template <> constexpr inline ValueType Construct::Typeof<Float>() noexcept  { return ValueType::Float; }
	template <> constexpr inline ValueType Construct::Typeof<String>() noexcept { return ValueType::String; }
	template <> constexpr inline ValueType Construct::Typeof<Array>() noexcept  { return ValueType::Array; }
};

std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);
