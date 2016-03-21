#pragma once

#include "pch.hpp"
#include "variant.hpp"
#include "types.hpp"
#include "Reference.hpp"
#include "Construct.hpp"

using namespace mapbox::util;

namespace Plang
{
    using List = std::vector<Reference>;
    using Dictionary = std::map<Reference, Reference>;
	using Pair = std::pair<Reference, Reference>;

    enum class ValueType
    {
        Invalid,
		Null,
        Int,
        Float,
        String,
        List,
		Tuple,
        Dictionary,
    };

	using ValueData = variant<
		std::nullptr_t,
		Int,
		Float,
		String,
		recursive_wrapper<Pair>,
		recursive_wrapper<List>,
		recursive_wrapper<Dictionary>,
		recursive_wrapper<Construct>
	>;

    class Value
    {
    public:
        Value() : type(ValueType::Invalid), data(nullptr), refCount(0) { }
        Value(ValueType Type, const ValueData& Data) : type(Type), data(Data), refCount(0) { }

        inline void AddRef() { refCount++; }
        inline void FreeRef() { refCount--; }
        inline size_t RefCount() const { return refCount; }

        template <class T>
        T& Data() { return data.get<T>(); }
        template <class T>
        T Data() const { return data.get<T>(); }

        ValueType type;

    protected:
        size_t refCount;
		ValueData data;
    };
};
