#pragma once

template<typename T>
class PtrHolder
{
protected:
	T* ptr;

public:
	PtrHolder() : ptr(nullptr) {}
	PtrHolder(T* ptr) : ptr(ptr) {}

	T* operator->()
	{
		return ptr;
	}

	const T* operator->() const
	{
		return ptr;
	}

	T* get()
	{
		return ptr;
	}

	const T* get() const
	{
		return ptr;
	}

	template<class TTrg>
	TTrg* cast()
	{
		return dynamic_cast<TTrg*>(ptr);
	}

	template<class TTrg>
	const TTrg* cast() const
	{
		return dynamic_cast<const TTrg*>(ptr);
	}

	operator bool() const
	{
		return ptr;
	}
};