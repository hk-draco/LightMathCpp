#pragma once

class StringView
{
private:
	const char* ptr;
	int length;

public:
	StringView() {}
	StringView(std::nullptr_t) : ptr(nullptr) {}
	explicit StringView(const char* ptr) : ptr(ptr) {}
	StringView(const char* ptr, int length) : ptr(ptr), length(length) {}

	IteratorRange<const char*> range() const
	{
		return IteratorRange(ptr, ptr + length);
	}

	char operator[] (int index) const
	{
		return ptr[index];
	}

	CompareResult compare(const StringView& trg)
	{
		if (length < trg.length)
			return CompareResult::Less;
		else if (length > trg.length)
			return CompareResult::Larger;

		for (int i = 0; i < length; i++) {
			if (ptr[i] < trg.ptr[i])
				return CompareResult::Less;
			else if (ptr[i] > trg.ptr[i])
				return CompareResult::Larger;
		}
		return CompareResult::Equal;
	}

	bool operator<(const StringView& trg) const
	{
		return compare(trg) == CompareResult::Less;
	}

	bool operator ==(const StringView& trg) const
	{
		if (length != trg.length)
			return false;
		auto res = memcmp(ptr, trg.ptr, sizeof(char) * length);
		return res == 0;
	}

	const char* data() const
	{
		return ptr;
	}

	void resize(int length)
	{
		this->length = length;
	}

	int size() const
	{
		return length;
	}
};

inline StringView operator"" _st(const char* str, size_t length)
{
	return StringView(str, length);
}

class String : public Vector<char, 64>
{
public:
	void append(char item)
	{
		push_back(item);
	}

	void append(StringView str)
	{
		push_back_range(str.range());
	}

	void append(const String& str)
	{
		push_back_range(str.range());
	}

	void append(const char* str)
	{
		for (auto c : ::range(str)) {
			push_back(c);
		}
	}

	template<typename T>
	void operator +=(T item)
	{
		append(item);
	}

	template<typename Iterator>
	void push_back_range(IteratorRange<Iterator> str)
	{
		for (auto c : str)
			push_back(c);
	}
};

IteratorRange<const char*> range(const char* str)
{
	return IteratorRange<const char*>(str, '\0');
}