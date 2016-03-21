#pragma once

#include "pch.hpp"
#include "Location.hpp"

namespace Plang
{
	class Exception
	{
	public:
		Exception() : parent(nullptr) { }
		Exception(const std::string& Message, const Location& Location, Exception* Parent = nullptr) : parent(Parent), message(Message) { }

		Exception* parent;
		std::string message;
		Location source;
		//callee?
	};
};