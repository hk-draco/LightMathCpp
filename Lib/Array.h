#pragma once

template<typename T, int max_size = 100>
class FixedArray
{
private:
	int _size;
	T ptr[max_size];

public:
	FixedArray() : _size(0) {}
	explicit FixedArray(int size) : _size(size) 
	{
		assert(0 <= size && size <= max_size);
	}

	FixedArray(FixedArray<T, max_size>&& src) = delete;
	FixedArray(const FixedArray<T, max_size>& src) = delete;

	template<int TSize>
	FixedArray(const FixedArray<T, TSize>& src) : _size(src.size())
	{
		memcpy(ptr, src.front(), sizeof(T) * _size);
	}

	FixedArray<T, max_size> null_removed()
	{
		FixedArray<T, max_size> res;
		for (auto item : range()) {
			if(item)
				res.push_back(item);
		}
		return res;
	}

	T& operator[](int index)
	{
		assert(0 <= index && index < _size);
		return ptr[index];
	}

	const T operator[](int index) const
	{
		assert(0 <= index && index < _size);
		return ptr[index];
	}

	void push_back(T item)
	{
		ptr[_size] = item;
		_size++;
	}

	void zero_clear()
	{
		ZeroMemory(data(), sizeof(T) * size());
	}

	void clear()
	{
		_size = 0;
	}
	
	void resize(int size)
	{
		assert(0 <= size && size <= max_size);
		_size = size;
	}

	T* data()
	{
		return ptr;
	}

	T pop_back()
	{
		_size--;
		return ptr[_size];
	}

	T front() const
	{
		return ptr[0];
	}

	T back() const
	{
		return ptr[_size - 1];
	}

	T& front()
	{
		return ptr[0];
	}

	T& back()
	{
		return ptr[_size - 1];
	}

	const T front() const
	{
		return ptr[0];
	}

	const T back() const
	{
		return ptr[_size - 1];
	}

	int size() const
	{
		return _size;
	}

	operator bool() const
	{
		return _size;
	}

	void remove(T trg) const
	{
		for (auto[item, idx] : range().indexed()) {
			if (item == trg) {
				int no_copy_len = idx + 1;
				memcpy(ptr + idx, ptr + no_copy_len, sizeof(T) * (_size - no_copy_len));
				return;
			}
		}
		THROW_EXCEPTION;
	}

	IteratorRange<T*> range() const
	{
		IteratorRange<T*>(ptr, ptr + _size);
	}
};

template<typename T>
using MemberArray = FixedArray<T, 1>;

template<typename T>
class ArrayBase
{
protected:
	int _size;
	T* ptr;

public:
	ArrayBase(T* ptr, int _size) : ptr(ptr), _size(_size) {}

	T& operator[](int idx)
	{
		assert(0 <= idx && idx < _size);
		return ptr[idx];
	}

	T operator[](int idx) const
	{
		assert(0 <= idx && idx < _size);
		return ptr[idx];
	}

	T* data()
	{
		return ptr;
	}

	const T front() const
	{
		return ptr[0];
	}

	const T back() const
	{
		return ptr[_size - 1];
	}

	int size() const
	{
		return _size;
	}

	operator bool() const
	{
		return _size;
	}

	IteratorRange<T*> range() const
	{
		IteratorRange<T*>(ptr, ptr + _size);
	}
};

template<typename T>
class HeapArray : public ArrayBase<T>
{
public:
	HeapArray() : ArrayBase(nullptr, 0) {}

	HeapArray(int size) : ArrayBase(::operator new(sizeof(T*) * size), size) {}
};

template<typename T>
class StackArray : public ArrayBase<T>
{
public:
	__forceinline StackArray(int size) : ArrayBase(alloca(sizeof(T) * size), size) {}
};

template<typename T>
class RefArray : public ArrayBase<T>
{
private:
	T* ptr;
	int _size;

public:
	RefArray() : _size(0) {}

	template<typename TArray>
	RefArray(const TArray& src) : ArrayBase(src.data(), src.size()) {}

	RefArray<T> next()
	{
		return RefArray<T>(ptr + 1, size - 1);
	}
};

template<typename TKey, typename TValue, typename TArray>
class Map
{
private:
	TArray buf;
	int _size;

public:
	__forceinline Map(int size) : buf(size) {}

	using pair_t = std::pair<TKey, TValue>;

	TValue& operator[](TKey key)
	{
		buf[_size - 1] = pair_t(Key, TValue());
		return buf[_size - 1].value;
	}

	TValue operator[](TKey key) const
	{
		for (int i = 0; i < _size; i++) {
			if (buf[i].key == key)
				return buf[i].value;
		}
		return TValue;
	}

	int size() const
	{
		return _size;
	}

	void resize(int size)
	{
		_size = size;
	}

	void clear()
	{
		_size = 0:
	}
};

template<typename TKey, typename TValue>
using StackMap = Map<TKey, TValue, StackArray<std::pair<TKey, TValue>>>;