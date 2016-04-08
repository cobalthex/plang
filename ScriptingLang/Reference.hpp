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

		template <class T>
		static Reference Create(const T& Construct) { return Reference(std::make_shared<T>(Construct)); }

		static Reference Undefined;
	};
};

std::ostream& operator << (std::ostream& Stream, const Plang::Reference& Reference);