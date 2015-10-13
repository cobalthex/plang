#pragma once
#include "pch.hpp"

//return the size in bytes of the largest type in the pack
template<typename F, typename... Ts> struct SizeofLargestType { static const size_t size = (sizeof(F) > SizeofLargestType <Ts...>::size ? sizeof(F) : SizeofLargestType <Ts...>::size); };
template<typename F> struct SizeofLargestType <F> { static const size_t size = sizeof(F); };

//A simple union wrapper that can store any value in Ty. Does not have any type-checking
//Note, manual deconstruction will be required
template <typename... Ts>
class Union
{
public:
	Union() = default;

	template <typename T>
	Union(const T& Value) { new (value) T(Value); }

	template <typename T>
	Union& operator = (const T& Value) { new (value) T(Value); return *this; }

	template <typename T>
	inline T& As() { return *(T*)value; }
	template <typename T>
	inline const T& As() const { return *(T*)value; }

	template <typename T>
	void Destroy() { ((T*)value)->~T(); }

private:
	uint8_t value[SizeofLargestType<Ts...>::size];
};