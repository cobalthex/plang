#pragma once

#include "pch.hpp"
#include "Value.hpp"

namespace Plang
{
    class Reference
    {
    public:
        Reference() : value(nullptr) { }
        Reference(Value* Value) : value(Value) { value->AddRef(); }
        Reference(const Reference& Reference) : value(Reference.value) { value->AddRef(); }
        Reference(Reference&& Reference) : value(std::move(Reference.value)) { }

        ~Reference() { value->FreeRef(); }

        Reference& operator = (const Reference& Reference)
        {
            if (this != &Reference)
            {
                value = Reference.value;
                value->AddRef();
            }
            return *this;
        }
        Reference& operator = (Reference&& Reference)
        {
            if (this != &Reference)
                value = std::move(Reference.value);
            return *this;
        }

        operator Value*() const
        {
            return value;
        }

        operator Value&() const
        {
            return *value;
        }

        Value* operator -> ()
        {
            return value;
        }

        static Reference Undefined;

    protected:
        Value* value;
    };
};
