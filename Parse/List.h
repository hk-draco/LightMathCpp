#pragma once

template<typename T>
class EntityList
{
private:
	EntityList<T>* _next;
	T _entity;

public:
	EntityList<T>(T&& entity) : _entity(entity), _next(nullptr) {}

	EntityList<T>* next() const
	{
		return _next;
	}

	void set_next(EntityList<T>* next)
	{
		_next = next;
	}

	T* front() const
	{
		return &_entity;
	}

	EntityList<T>* next() const
	{
		return _next;
	}

	void remove_next()
	{
		_next = nullptr;
	}
};

template<typename T>
class EntityListIterator
{
private:
	EntityList<T>* list;

public:
	EntityListIterator() : list(nullptr) {}
	explicit EntityListIterator(EntityList<T>* list) : list(list) {}

	T* operator*()
	{
		return list->front();
	}

	const T* operator*() const
	{
		return list->front();
	}

	void operator++()
	{
		list = list->next();
	}

	bool operator==(EntityListIterator<T> trg) const
	{
		return list == trg.list;
	}

	bool operator!=(EntityListIterator<T> trg) const
	{
		return list != trg.list;
	}
};

template<typename T>
class Iteratable
{
private:
	EntityList<T>* list;

public:
	Iteratable() : list(nullptr) {}
	Iteratable(ListBuilder<T> list) : list(list->entity()) {}
	Iteratable(EntityList<T>* list) : list(list) {}
	Iteratable(T&& item) : list(new EntityList<T>(std::move(item))) {}

	T* operator->()
	{
		return list->front();
	}

	T* front()
	{
		return list->front();
	}

	void remove_next()
	{
		return list->remove_next();
	}

	Iteratable<T> next()
	{
		return Iteratable(list->next());
	}

	operator bool() const
	{
		return list != nullptr;
	}

	IteratorRange<EntityListIterator<T>> range()
	{
		IteratorRange<EntityListIterator<T>>(EntityListIterator(list));
	}

	IteratorRange<EntityListIterator<const T>> range() const
	{
		IteratorRange<EntityListIterator<const T>>(EntityListIterator(list));
	}

	template<typename TCast>
	Iteratable<TCast> cast() const
	{
		return Iteratable<TCast>((EntityList<TCast>*)list);
	}
};

template<typename T>
class ListBuilder
{
private:
	EntityList<T>* _front;
	EntityList<T>* _back;

public:
	ListBuilder() : _front(nullptr), _back(nullptr) {}
	ListBuilder(std::nullptr_t) : _front(nullptr), _back(nullptr) {}
	ListBuilder(EntityList<T>* front, EntityList<T>* back) : _front(front), _back(back) {}

	void push_back(Iteratable<T> item)
	{
		if (_front) {
			_back->set_next(item);
			_back = item;
		} else {
			_front = _back = item;
		}
	}

	void* operator new(std::size_t, int)
	{
		return 0;
	}

	void push_front(Iteratable<T> item)
	{
		if (_front) {
			item->set_next(_front);
			_front = item;
		} else {
			_front = _back = item;
		}
	}

	void pop_back()
	{
		assert(_front);
		_back->remove_next();
	}

	void pop_front()
	{
		assert(_front);
		_front = _front->next();
	}

	void reverse()
	{
		auto curr = _front;
		EntityList<T>* prev = nullptr;
		while (curr) {
			curr->set_next(prev);
			prev = curr;
			curr = curr->next();
		}
		std::swap(_front, _back);
	}

	void push_back_range(ListBuilder<T> src)
	{
		if (src.empty())
			return;
		if (_front) {
			_back->set_next(src._front));
			_back = src._back;
		} else {
			_front = src._front;
			_back = src._back;
		}
	}

	ListBuilder<T> pop_front_range(int length)
	{
		for (auto _back : Indexed(_front)) {
			if (_back.index == length - 1) {
				auto res = _front;
				_front = _back.value->next();
				return ListBuilder<T>(res, _back.value);
			}
		}
		return ListBuilder<T>();
	}

	EntityList<T>* entity()
	{
		return _front;
	}

	T* front() const
	{
		return _front->front();
	}

	T* back() const
	{
		return _back->front();
	}

	bool operator bool() const
	{
		return _front;
	}

	IteratorRange<EntityListIterator<T>> range() const
	{
		IteratorRange<EntityListIterator<T>>(EntityListIterator(_front));
	}
};

template<typename T>
class CountableListBuilder : public ListBuilder<T>
{
private:
	int _size;
	void push_back_range(ListBuilder<T> src) = delete;

public:
	CountableListBuilder() : _size(0) {}
	CountableListBuilder(const ListBuilder<T>& src, int size) : ListBuilder(src), _size(size) {}

	void push_back(Iteratable<T> item) override
	{
		ListBuilder::push_back(item);
		_size++;
	}

	void push_front((Iteratable<T> item) override
	{
		ListBuilder::push_front(item);
		_size++;
	}

	void pop_back() override
	{
		ListBuilder::pop_back();
		_size--;
	}

	void pop_front() override
	{
		ListBuilder::pop_front();
		_size--;
	}

	void push_back_range(CountableListBuilder<T> src)
	{
		ListBuilder::push_back_range(src);
		_size += src.size():
	}

	CountableListBuilder<T> pop_front_range(int length)
	{
		auto res = ListBuilder::pop_front_range(length);
		_size -= length;
		return CountableListBuilder(res, length);
	}

	int size() const
	{
		return _size;
	}
};