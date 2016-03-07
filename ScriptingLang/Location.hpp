#pragma once

#include "pch.hpp"

struct Location
{
	std::string module;
	size_t line;
	size_t column;

	inline operator std::string() const
	{
		return module + " (" + std::to_string(line) + ", " + std::to_string(column) + ")";
	}
};

inline std::ostream& operator << (std::ostream& Stream, const Location& Location)
{
	Stream << Location.module << " (" << Location.line << ", " << Location.column << ")";
	return Stream;
}