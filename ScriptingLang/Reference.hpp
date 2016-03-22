#pragma once

#include "pch.hpp"

namespace Plang
{
	class Construct;

    class Reference
    {
    public:
        Reference() : value(nullptr) { }
		Reference(Value* Value);
		Reference(const Reference& Reference);
		Reference(Reference&& Reference);

		~Reference();

		Reference& operator = (const Reference& Reference);
		Reference& operator = (Reference&& Reference);

        inline operator Value*() const
        {
            return value;
        }

        inline operator Value&() const
        {
            return *value;
        }

        inline Value* operator -> ()
        {
            return value;
        }

        static Reference Undefined;

    protected:
        Construct* construct;
    };
};
