#pragma once

#include "pch.hpp"

namespace Plang
{
	class Construct;

	template <class T>
	class Reference
	{
	public:
		inline Reference() : count(nullptr), ptr(nullptr) { }
		inline Reference(const T& Value) : count(new size_t(1)), ptr(new T(Value)) { }
		inline Reference(const Reference& Ref) : count(Ref.count), ptr(Ref.ptr) { AddRef(); }
		inline Reference(Reference&& Ref) : count(Ref.count), ptr(Ref.ptr) { Ref.ptr = nullptr; Ref.count = nullptr; }

		inline Reference& operator = (const Reference& Ref)
		{
			if (ptr != Ref.ptr)
			{
				FreeRef();
				ptr = Ref.ptr;
				count = Ref.count;
				AddRef();
			}
			return *this;
		}
		inline Reference& operator = (Reference&& Ref)
		{
			if (ptr != Ref.ptr)
			{
				FreeRef();
				ptr = Ref.ptr;
				count = Ref.count;

				Ref.ptr = nullptr;
				Ref.count = nullptr;
			}
			return *this;
		}

		inline ~Reference()
		{
			FreeRef();
		}

		inline T& operator *() { return *ptr; }
		inline const T& operator *() const { return *ptr; }

		inline T* operator ->() { return ptr; }
		inline const T* operator ->() const { return ptr; }

		inline size_t RefCount() const { return *count; }

		template <class U>
		inline operator Reference<U>() noexcept
		{
			Reference<U> u;
			u.ptr = static_cast<U*>(ptr);
			u.count = count;
			u.AddRef();
			return u;
		}
		
	protected:
		//assumes reference is not nullptr
		inline void AddRef() { if (count != nullptr) (*count)++; }
		inline void FreeRef()
		{
			if (count != nullptr && ptr != nullptr && --(*count) < 1)
			{
				delete ptr;
				ptr = nullptr;
				delete count;
				count = nullptr;
			}
		}

		size_t* count;
		T* ptr;

		template <typename>
		friend class Reference;
	};

	using AnyRef = Reference<Construct>;
	static AnyRef Undefined;
	static AnyRef Null;
};