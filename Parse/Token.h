#pragma once
#include "Location.h"

//======================================================================
// 8ビット文字8文字分を64ビット整数で表現する関数
//======================================================================
using Ident = std::uint64_t;

constexpr Ident to_uint64(const char* str, size_t offset, Ident U64 = 0)
{
	return offset ? to_uint64(str, offset - 1, (U64 << CHAR_BIT) | str[offset - 1]) : U64;
}

constexpr Ident operator"" _u64(const char* str, size_t length)
{
	return to_uint64(str, length);
}

// 強制コンパイル時constexpr
#define CTS(STR) std::integral_constant<Ident, STR##_u64>::value

//======================================================================
// 演算子の性質を表現するクラス
//======================================================================
class Operator
{
private:
	struct Token* func_name;
	int arg_num;
	int preced;
	bool is_func;
	bool is_left_assoc;

public:
	Operator() : func_name(nullptr) {}
	Operator(struct Token* func_name, int arg_num, int preced, bool is_left_assoc)
		: func_name(func_name), arg_num(arg_num), preced(preced), is_left_assoc(is_left_assoc), is_func(false)
	{}
	Operator(struct Token* func_name)
		: func_name(func_name), arg_num(1), preced(0), is_left_assoc(true), is_func(true)
	{}

	struct Token* get_func_name() const
	{
		return func_name;
	}

	void increment_arg()
	{
		arg_num++;
	}

	bool get_is_func() const
	{
		return is_func;
	}

	int get_arg_num() const
	{
		return arg_num;
	}

	bool preceder(const Operator& trg) const
	{
		if (preced == 0)
			return true;
		return preced > trg.preced || (trg.is_left_assoc && preced == trg.preced);
	}
};

//======================================================================
// トークン
//======================================================================
enum class Annotator
{
	BuildIn, Empty, EndLine, Symbol, Operator, String, Char, Identifer, Integer, Decimal, Error // 最適化した順番
};

struct TokenString
{
	StringView str;
	union
	{
		long long int_value;
		double float_value;
		Ident ident;
	};

	TokenString() {}
	TokenString(StringView str) : str(str) {}

	CompareResult compare(const TokenString& trg)
	{
		if (ident) {
			if (!trg.ident)
				return CompareResult::Less;
			return ::compare(ident, trg.ident);
		} else if (trg.ident) {
			return CompareResult::Larger;
		} else {
			return str.compare(trg.str);
		}
	}

	bool operator<(const TokenString& trg)
	{
		return compare(trg) == CompareResult::Less;
	}

	bool operator=(const TokenString& trg)
	{
		if (ident != trg.ident)
			return false;
		if (ident)
			return ident == trg.ident;
		return str == trg.str;
	}
};

struct Token : TokenString
{
	Annotator ann;
	Location loc;
	Operator* ope;

	Token() {}
	Token(StringView str, Annotator ann, Location loc)
		: TokenString(str), ann(ann), loc(loc)
	{}

	Location begin() const
	{
		return loc;
	}

	Location end() const
	{
		return Location(loc.row, loc.column + str.size());
	}

	Area area() const
	{
		return Area(begin(), end());
	}
};
