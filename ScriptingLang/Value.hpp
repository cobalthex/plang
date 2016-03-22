#pragma once

#include "pch.hpp"
#include "variant.hpp"
#include "types.hpp"

using namespace mapbox::util;

namespace Plang
{
    enum class ValueType
    {
        Invalid,
		Null,
		Bool,
        Int,
        Float,
        String,
    };

	using Primative = variant<void*, std::nullptr_t, bool, Int, Float, std::string>; //holds directly any primatives, or a pointer to complex data (arrays, etc)

	class Value
	{
		size_t count;
		Primative data;
	};
};
