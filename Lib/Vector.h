#pragma once

template<typename T, int block_size>
class VectorIterator
{
private:
	Iteratable<T[block_size]> itr;
	int idx;

public:
	VectorIterator(Iteratable<T[block_size]> itr, int idx) : itr(itr), idx(idx) {}

	void operator++()
	{
		if (idx >= block_size) {
			itr = itr.next();
			idx = 0;
		} else {
			idx++;
		}
	}

	T operator*()
	{
		return (*itr.front())[idx];
	}

	bool operator!=(VectorIterator<T, block_size> trg) const
	{
		return itr != trg.itr || idx != trg.idx;
	}
};

template<typename T, int block_size>
class Vector
{
private:
	ListBuilder<Block> buf;
	int block_num;
	int curr_num;

public:
	using Block = T[block_size];
	using Iterator = VectorIterator<T, block_size>;

	Vector() : block_num(0), curr_num(block_size) {}

	void push_back(T item)
	{
		if (curr_num >= block_size) {
			buf.push_back(new EntityList(Block()));
			block_num++;
		}

		(*buf.back())[curr_num] = item;
		curr_num++;
	}

	int size() const
	{
		return (block_num - 1) * block_size + curr_num:
	}

	IteratorRange<Iterator> range() const
	{
		auto end = curr_num >= block_size ? Iterator(nullptr, 0) : Iterator(buf.back(), curr_num);
		return IteratorRange(Iterator(buf.front(), 0), end);
	}

	operator bool() const
	{
		return block_num || curr_num < block_num;
	}
};