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
        Int,
        Float,
        String,
        List,
        Dictionary,
    };

    using ValueData = mapbox::util::variant<decltype(nullptr), Int, Float, String, List, Dictionary>; //use string ptrs (StringPool ?)

    class Value
    {
    public:
        Value() : type(ValueType::Invalid), refCount(0) { }
        Value(ValueType Type, const ValueData& Data) : type(Type), data(Data), refCount(0) { }

        inline void AddRef() { refCount++; }
        inline void FreeRef() { refCount--; }
        inline size_t RefCount() const { return refCount; }

        template <class T>
        T& Get() { return data.get<T>(); }
        template <class T>
        T Get() const { return data.get<T>(); }

        ValueData data;
        ValueType type;

        static Value* const Undefined;

    protected:
        size_t refCount;
    };
};
