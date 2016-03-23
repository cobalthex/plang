#pragma once

#include "pch.hpp"
#include "Reference.hpp"

namespace Plang
{
	class Scope;

	using Primative = variant<void*, std::nullptr_t, bool, Int, Float, std::string>; //holds directly any primatives, or a pointer to complex data (arrays, etc)


	//The basic object. Everything is a construct from literals to lexical blocks to functions
	class Construct
	{
	public:
		Construct() { }
		~Construct() { }

		bool IsCallable() const;
		Plang::Reference Call(const Plang::Reference& Arguments, Scope* DotScope = nullptr, Scope* LexScope = nullptr); //Passing Specific scopes predefines the scopes. No automatic inheritance is applied with custom scopes

	protected:
		Scope* lexScope; //the local (lexical) scope. Inherits from the lexical scope of the enclosing block. Created on call (or instantiation)
		Scope* dotScope; //the public property scope. Inherits from accessor that calls this. Created on call (or instantiation)

		Primative value;
	};
};
