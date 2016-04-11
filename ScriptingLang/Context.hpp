#pragma once

#include "pch.hpp"
#include "Scope.hpp"

namespace Plang
{
	//Provices an isolated environment for a runtime
	class Context
	{
	public:
		Context() = default;

		Scope global; //the global scope, the parent scope of this is intrinsic

		static Scope intrinsic;
	};
};