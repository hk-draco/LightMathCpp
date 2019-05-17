#pragma once

template<typename T>
struct IndexedValue
{
	T value;
	int index;

	IndexedValue(T value, int index) : value(value), index(index) {}
};

template<typename Iterator>
class IndexedIterator
{
private:
	Iterator itr;
	int index;

public:
	IndexedIterator() : list(nullptr) {}
	explicit IndexedIterator(Iterator itr) : itr(itr), index(0) {}

	auto operator*()
	{
		return IndexedValue(*itr, index);
	}

	auto operator*() const
	{
		return IndexedValue(const *itr, index);
	}

	void operator++()
	{
		itr++;
		index++;
	}

	bool operator==(IndexedIterator<T> trg) const
	{
		return itr == trg.itr;
	}

	bool operator!=(IndexedIterator<T> trg) const
	{
		return itr != trg.itr;
	}
};

template<typename Iterator1, typename Iterator2>
class ZipIterator
{
private:
	Iterator1 itr1;
	Iterator2 itr2;

public:
	ZipIterator() : list1(nullptr), list2(nullptr) {}
	ZipIterator(Iterator1 itr1, Iterator2 itr2) : itr1(itr1), itr2(itr2) {}

	auto operator *()
	{
		return std::pair(*itr1, *itr2);
	}

	void operator++()
	{
		itr1++;
		itr2++;
	}

	bool operator==(ZipIterator<Iterator1, Iterator2> trg) const
	{
		return itr1 == trg.itr1 && itr2 == trg.itr2;
	}

	bool operator!=(ZipIterator<Iterator1, Iterator2> trg) const
	{
		return itr1 != trg.itr1 || itr2 != trg.itr2;
	}
};

template<typename Iterator1, typename Iterator2>
class RefZipIterator : public ZipIterator<Iterator1, Iterator2>
{
public:
	RefZipIterator() : {}
	RefZipIterator(Iterator1 itr1, Iterator2 itr2) : RefZipIterator(itr1, itr2) {}

	auto operator *()
	{
		return std::pair(&*itr1, &*itr2);
	}
};

template<typename T>
class ReverseIterator
{
private:
	T* ptr;

public:
	ReverseIterator(T* ptr) : ptr(ptr) {}

	T& operator*()
	{
		return *ptr;
	}

	void operator++()
	{
		ptr--;
	}

	bool operator==(ReverseIterator<T> trg) const
	{
		return ptr == trg.ptr;
	}

	bool operator!=(ReverseIterator<T> trg) const
	{
		return ptr != trg.ptr;
	}
};

template<typename Iterator>
class IteratorRange
{
private:
	Iterator _begin;
	Iterator _end;

public:
	IteratorRange(Iterator begin, Iterator end = Iterator()) : begin(begin), end(end) {}

	Iterator begin() const
	{
		return _begin;
	}

	Iterator end() const
	{
		return _end;
	}

	IteratorRange<IndexedIterator<Iterator>> indexed()
	{
		return IteratorRange<IndexedIterator<Iterator>>(IndexedIterator(_begin), IndexedIterator(_end));
	}

	IteratorRange<Iterator> cut_back()
	{
		return IteratorRange<Iterator>(_begin, --_end);
	}

	auto reversed()
	{
		return IteratorRange<ReverseIterator<decltype(*Iterator())>>(_end - 1, _begin - 1);
	}

	int count() const
	{
		int size = 0;
		for (auto item : (*this))
			size++;
		return size;
	}
};

template<typename Range1, typename Range2>
auto Zip(Range1 range1, Range2 range2)
{
	return IteratorRange(ZipIterator(range1.begin(), range2.begin()), ZipIterator(range1.end(), range2.end()));
}

template<typename Range1, typename Range2>
auto RefZip(Range1 range1, Range2 range2)
{
	return IteratorRange(RefZipIterator(range1.begin(), range2.begin()), RefZipIterator(range1.end(), range2.end()));
}