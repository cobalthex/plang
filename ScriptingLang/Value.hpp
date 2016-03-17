#pragma once

#include "pch.hpp"
#include "variant.hpp"
#include "types.hpp"

namespace Plang
{
    class Reference;

    using List = std::vector<Reference>;
    using Dictionary = std::map<String, Reference>;

    enum class ValueType
    {
        Invalid,
        Binary,
        Int,
        Float,
        String,
        List,
        Dictionary,
    };

    using ValueData = mapbox::util::variant<Int, Float, String, List, Dictionary>; //use string ptrs (StringPool ?)


    class Value
    {
    public:
        Value() : type(Invalid), refCount(0) { }
        Value(ValueType Type, const ValueData& Data) : type(Type), data(Data), refCount(0) { }

        inline void AddRef() { refCount++; }
        inline void FreeRef() { refCount--; }
        inline size_t RefCount() const { return refCount; }

        ValueData data;
        ValueType type;

    protected:
        size_t refCount;
    };

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

        const Reference Undefined;

    protected:
        Value* value;
    };
};
