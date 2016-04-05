#pragma once

#include "pch.hpp"

namespace Plang
{
	class Construct;

	class Reference : public std::shared_ptr<Construct>
	{
	public:
		Reference() : std::shared_ptr<Construct>(nullptr) { }
		Reference(const std::shared_ptr<Construct>& Ptr) : std::shared_ptr<Construct>(Ptr) { }

		static Reference Undefined;
	};
};