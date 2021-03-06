// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "memory.h"
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN    // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>

#include <map>
#include <vector>
#include <set>
#include <array>
#include <string>
#include <cassert>
#include <combaseapi.h>
#include "Base.h"
#include "Allocator.h"
#include "Iterator.h"
#include "List.h"
#include "Array.h"
#include "Vector.h"
#include "String.h"
#include "Pointer.h"

#define THROW_EXCEPTION assert(false)

#undef max
#undef min

template<typename T, int length>
class Array
{
private:
	T buf[length];
	int count;

public:
	void push_back(T obj)
	{
		buf[count - 1] = obj;
	}

	T& operator[] (int idx)
	{
		return buf[idx];
	}

	T operator[] (int idx) const
	{
		return buf[idx];
	}

	T &front()
	{
		return buf[0];
	}

	T front() const
	{
		return buf[0];
	}

	T &back()
	{
		return buf[count - 1];
	}

	T back() const
	{
		return buf[count - 1];
	}

	int size() const
	{
		return count;
	}

	T *begin()
	{
		return buf;
	}

	T *end()
	{
		return buf + count;
	}
};

template<typename T>
struct Vector : public std::vector<std::unique_ptr<T>>
{
	std::vector<T*> get() const
	{
		std::vector<T*> res;
		res.reserve(this->size());
		for (auto& item : *this)
			res.push_back(item.get());
		return res;
	}
};

template<typename T>
class ArrayPtr
{
private:
	const T *_begin;
	const T *_end;

public:
	template<int size>
	ArrayPtr(const Array<T, size> &src) : _begin((const T*)src.begin()), _end((const T*)src.end()) {}
	ArrayPtr(const Vector<T> &src) : _begin((const T*)&src.front()), _end((const T*)&src.end() + 1) {}
	ArrayPtr(const std::vector<T> &src) : _begin((const T*)&src.front()), _end((const T*)&src.end() + 1) {}
	ArrayPtr(T *_begin, T *_end) : _begin(_begin), _end(_end) {}

	T operator[] (int idx) const
	{
		return _begin[idx];
	}

	int size() const
	{
		return _end - _begin;
	}

	const T *begin() const
	{
		return _begin;
	}

	const T *end() const
	{
		return _end;
	}
};

inline void write_int(std::string *buf, int value)
{
	if (value == 0) {
		buf->push_back('0');
		return;
	}

	if (value < 0) {
		buf->push_back('-');
		value *= -1;
	}

	int num = 1;
	for (; value / num > 0; num *= 10);
	for (num /= 10; num > 0; num /= 10) {
		int os = value / num;
		value -= os * num;
		buf->push_back('0' + os);
	}
}

/*
template<typename T>
class UnpushableUtilList
{
private:
	T *ptr;

public:

};

UnpushableUtilList<T> get()
{
	for (auto expr : Range(next.get()))
		expr->util_next = expr->next.get();
}

template<typename T>
class CountabbleUtilList : public UtilList<T>
{
private:
	int count;

public:
	CountabbleUtilList() : count(0) {}

	void push_back(T *src) override
	{
		push_back(src);
		count++;
	}

	int size() const
	{
		return count;
	}
};





void Expression::write(std::string* buf, ExpressionShowMode mode) const
{
	switch (type) {
	case ExpressionType::Step:
		switch (mode) {
		case ExpressionShowMode::Latest:
			args.back()->write(buf, mode);
			break;
		default:
		{
			buf->push_back('[');
			bool is_first = true;
			for (auto arg : args.range()) {
				if (is_first)
					is_first = false;
				else
					buf->append(", ");
				arg->write(buf, mode);
			}
			buf->push_back(']');
		}
		}
		break;
	case ExpressionType::Constant:
	case ExpressionType::Variable:
		head->str.write(buf);
		break;

	case ExpressionType::Lambda:
		//bind_var->write(buf); TODO
		buf->append(": ");
		args.front()->write(buf, mode);
		break;

	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:
		if (head->ope.get_is_func() || args.empty()) {
			head->str.write(buf);
			buf->push_back('(');
			bool is_first = true;
			for (auto arg : args.range()) {
				if (is_first)
					is_first = false;
				else
					buf->append(", ");
				arg->write(buf, mode);
			}
			buf->push_back(')');
		} else if (!args.front_next()) {
			head->str.write(buf);
			auto arg = args.front();
			if (arg->is_func() && (!arg->head->ope.get_is_func() || head->ope.preceder(arg->head->ope))) {
				buf->push_back('(');
				arg->write(buf, mode);
				buf->push_back(')');
			} else {
				arg->write(buf, mode);
			}
		}  else {
			bool is_first = true;
			for (auto arg : Range(args)) {
				if (is_first)
					is_first = false;
				else
					head->str.write(buf);
				if (arg->is_func() && (!arg->head->ope.get_is_func() || head->ope.preceder(arg->head->ope))) {
					buf->push_back('(');
					arg->write(buf, mode);
					buf->push_back(')');
				} else {
					arg->write(buf, mode);
				}
			}
		}
	default:
		break;
	}
}


void parse_declaration_normal(Area area, DeclarationInfo info)
{
	auto dec = Iteratable(Declaration(info.type, area));
	bool has_error = false;

	// 識別子
	dec->name = nullptr;
	if (info.need_ident && !(dec->name = state.maybe_ident())) {
		state.make_error_curr("識別子がありません");
		has_error = true;
	}

	// 変数宣言
	if (state.maybe_token(CTS("("))) {
		dec->var_dec = parse_var_dec(Quantifier::ForAll);
		if (!state.expect_token(CTS(")"), CTS("}")))
			return;
	}

	// 型指定
	if (info.is_type) {

	} else {
		if (state.maybe_token(CTS(":"))) {
			dec->eval_type = state.parse_expression();
		} else if (info.need_type) {
			state.make_error_curr("型指定が必要です");
			has_error = true;
		}
	}

	if (state.maybe_token(CTS("{"))) {
		auto stms = state.parse_statements();
		dec->blocks = create_blocks(stms);
		state.expect_token(CTS("}"), CTS("\0"));
	}

	if (!has_error)
		decs.push_back(dec);
}

*/
