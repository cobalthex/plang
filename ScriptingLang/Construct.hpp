#pragma once

#include "pch.hpp"
#include "Reference.hpp"
#include "Scope.hpp"

namespace Plang
{
	class Construct
	{
	public:
		Construct() { }

		bool IsCallable() const;
		Plang::Reference Call(const Plang::Reference& Arguments, Scope* DotScope = nullptr, Scope* LexicalScope = nullptr);

	protected:
		Scope* lexicalScope;
		Scope* dotScope;
	};
}