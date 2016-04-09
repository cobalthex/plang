#pragma once

#include "pch.hpp"

template <class T>
class Array
{
public:
	Array() : indices(nullptr), length(0) { }

	Array(size_t Length) : indices(new T[Length]), length(Length) { }
	Array(size_t Length, const T& Value) : indices(new T[Length]()), length(Length) { std::fill(indices, indices + length, Value); }

	Array(T* Array, size_t Length) : indices(new T[Length]), length(Length) { std::copy(Array, Array + Length, indices); }
	// Array(T Array[]) : Array(Array, sizeof(Array) / sizeof(T)) { }

	Array(const Array& Array) : indices(new T[Array.length]), length(Array.length) { std::copy(Array.indices, Array.indices + Array.length, indices); }
	Array(Array&& Array) : indices(Array.indices), length(Array.length) { Array.indices = nullptr; Array.length = 0; }

	~Array() { if (indices != nullptr) delete[] indices; length = 0; }

	inline Array& operator = (const Array& Array)
	{
		if (Array.indices != indices)
		{
			if (indices != nullptr)
				delete[] indices;

			std::copy(Array.indices, Array.indices + Array.length, indices);
			length = Array.length;
		}
		return *this;
	}
	inline Array& operator = (Array&& Array)
	{
		if (Array.indices != indices)
		{
			if (indices != nullptr)
				delete[] indices;

			indices = Array.indices;
			length = Array.length;

			Array.indices = nullptr;
			Array.length = 0;
		}
		return *this;
	}

	inline operator T*() noexcept { return indices; }
	inline operator const T*() const noexcept { return indices; }
	inline T* Data() noexcept { return indices; }
	inline const T* Data() const noexcept { return indices; }

	inline size_t Length() const noexcept { return length; }

	inline T& operator [](size_t Index) { return indices[Index]; }
	inline const T& operator [](size_t Index) const { return indices[Index]; }
	inline T& At(size_t Index) { return indices[Index]; }
	inline const T& At(size_t Index) const { return indices[Index]; }

protected:
	T* indices;
	size_t length;
};
