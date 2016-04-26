#pragma once

#include "pch.hpp"

template <class T>
class Array
{
public:
	Array() : length(0), indices(nullptr) { }

	Array(size_t Length) : length(Length), indices(new T[Length]) { }
	Array(size_t Length, const T& Value) : length(Length), indices(new T[Length]) { std::fill(indices, indices + length, Value); }
	Array(std::initializer_list<T> Indices) : length(Indices.size()), indices(new T[Indices.size()]) { std::copy(Indices.begin(), Indices.end(), indices); }

	Array(T* Array, size_t Length) : length(Length), indices(new T[Length]) { std::copy(Array, Array + Length, indices); }
	Array(T Array[]) : length(sizeof(Array) / sizeof(T)), indices(Array) { }

	Array(const Array& Array) : length(Array.length), indices(new T[Array.length]) { std::copy(Array.indices, Array.indices + Array.length, indices); }
	Array(Array&& Array) : length(Array.length), indices(Array.indices) { Array.indices = nullptr; Array.length = 0; }

	~Array() { if (indices != nullptr) delete[] indices; length = 0; }

	/*Iterator begin() { return Iterator(indices); }
	ConstIterator begin() const { return Iterator(indices); }
	Iterator end() { return Iterator(indices + length); }
	const ConstIterator cend() const { return Iterator(indices + length); }*/

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
	constexpr operator const T*() const noexcept { return indices; }
	inline T* Data() noexcept { return indices; }
	constexpr const T* Data() const noexcept { return indices; }

	constexpr size_t Length() const noexcept { return length; }

	inline T& operator [](size_t Index) { return indices[Index]; }
	constexpr const T& operator [](size_t Index) const { return indices[Index]; }
	inline T& At(size_t Index) { return indices[Index]; }
	constexpr const T& At(size_t Index) const { return indices[Index]; }

	//Create a copy of all or part of this array. Zero slices to end
	inline ::Array<T> Slice(size_t Start, size_t Length = 0) const
	{
		size_t len = Length > 0 ? std::min(length - Start, Length) : (Length - Start);
		return ::Array<T>(indices + Start, len);
	}

protected:
	size_t length;
	T* indices;
};
