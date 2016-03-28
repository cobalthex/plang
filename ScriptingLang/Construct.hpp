#pragma once

#include "pch.hpp"
#include "types.hpp"

namespace Plang
{
	class Scope;
	class Construct;
	using Reference = std::shared_ptr<Construct>;

	using Primative = mapbox::util::variant<void*, std::nullptr_t, bool, Int, Float, std::string>; //holds directly any primatives, or a pointer to complex data (arrays, etc)
	using Array = std::vector<Reference>;

	enum class ValueType
	{
		Unknown,
		Null,
		Bool,
		Int,
		Float,
		String,
		Array,
	};

	//The basic object. Everything is a construct from literals to lexical blocks to functions
	class Construct
	{
	public:
		Construct() { }
		~Construct() { }

		template <typename T>
		inline T& Get() const { return value.get<T>(); }
		template <typename T>
		inline T& Get() { return value.get<T>(); }

		template <>
		inline Array& Get() const { return *(Array*)value.get<void*>(); }

		template <typename T>
		inline void Set(const T& Value) { value.set<T>(Value); }

		bool IsCallable() const;
		//Plang::Reference Call(const Plang::Reference& Arguments, Scope* DotScope = nullptr, Scope* LexScope = nullptr); //Passing Specific scopes predefines the scopes. No automatic inheritance is applied with custom scopes

	protected:
		Scope* lexScope; //the local (lexical) scope. Inherits from the lexical scope of the enclosing block. Created on call (or instantiation)
		Scope* dotScope; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)

		Primative value;
	};

	static Reference Undefined;
};
std::ostream& operator << (std::ostream& Stream, const Plang::Construct& Construct);