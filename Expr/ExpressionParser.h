#pragma once
#include "Token.h"
#include "Expression.h"
#include "Message.h"
#include "ParsingState.h"

class ExpressionParseState
{
private:
	FixedArray<ExpressionRef> exprs;
	FixedArray<Token*> opes;
	MessageManager* msg;
	int less_term_num;

	void push_expr(ExpressionRef expr)
	{
		exprs.push_back(expr);
		less_term_num--;
	}

	void make_expr(Token* ope)
	{
		int arg_num = ope->ope.get_arg_num();
		less_term_num++; // pop stack

		auto expr = Expression::create(ope, arg_num);

		for (int i = 0; i < arg_num; i++) {
			if (!exprs) {
				msg->add_err(std::to_string(i) + "引数を受け取る$O0はありません", ope->area());
				break;
			}

			expr->set_arg(arg_num - i, exprs.pop_back());
		}

		push_expr(expr);
	}

public:
	ExpressionParseState(MessageManager* msg) : msg(msg), less_term_num(1) {}

	// 項の追加
	void add_term(Token* token);

	// 関数追加(引数の数は暫定1個)
	void add_func(Token* token)
	{
		token->ope = Operator(token);
		opes.push_back(token);
	}

	// 演算子追加(既に引数の数は確定している)
	void add_ope(Token* token)
	{
		opes.push_back(token);
		less_term_num += token->ope.get_arg_num();
		less_term_num--; // add stack
	}

	// 括弧の追加
	void add_bracket(Token* token)
	{
		token->ope = Operator(token);
		opes.push_back(token);
		less_term_num++; // === "("
	}

	// タプル用及び優先用の括弧の追加(コンマによって引数を確定する)
	void add_tuple_bracket(Token* token)
	{
		token->ope = Operator(token);
		opes.push_back(token);
		less_term_num++; // === "("
	}

	// 最も新しい関数の引数の数を増やす
	void increment_func_arg()
	{
		for (auto ope : opes.range().reversed()) {
			if (ope->ope.get_is_func()) {
				ope->ope.increment_arg();
				break;
			}
		}
		less_term_num++;
	}

	// 演算子スタックの先頭が開き括弧になるまで式を作成する
	void make_expr_to_bracket()
	{
		while (opes) {
			if (opes.back()->ident == CTS("("))
				break;
			else
				make_expr(opes.pop_back());
		}
	}

	// 演算子スタックの先頭の括弧をポップするかタプルを作成する
	void pop_bracket()
	{
		if (!opes || opes.back()->ident != CTS("(")) {
			msg->add_err("括弧が揃っていません", exprs.back()->get_area());
			return;
		}
		if (opes.back()->ope.get_arg_num() > 1) {
			opes.back()->ident = CTS("tuple");
			make_expr(opes.back());
		}
		opes.pop_back();
		less_term_num--; // === ")"
	}

	// 演算子スタックの先頭から任意の演算子より優先順位の高い演算子の全ての式を作成
	void make_expr_to_preceder(const Operator& ope)
	{
		while (opes) {
			if (opes.back()->ident != CTS("(") && opes.back()->ope.preceder(ope)) {
				make_expr(opes.back());
				opes.pop_back();
			} else {
				break;
			}
		}
	}

	// 全ての式を作成する
	void make_expr_all()
	{
		for (auto ope : opes.range().reversed()) {
			if (ope->ident == CTS("("))
				msg->add_err("括弧が揃っていません", ope->area());
			make_expr(ope);
		}
	}

	// 式が終了しているかどうか
	bool is_ended()
	{
		// 終了条件: スタックの演算子の引数の合計 - スタックの演算子の数 + 1 = 出力スタックの項の数
		// less_term_num = スタックの演算子の引数の合計 - スタックの演算子の数 - 出力スタックの項の数 + 1
		return less_term_num == 0;
	}

	ExpressionRef get_expr()
	{
		return exprs.back();
	}
};

class ExpressionParser : protected ExpressionParseState
{
private:
	ExpressionParser(MessageManager* msg) : ExpressionParseState(msg) {}

	ExpressionRef parse(ParsingState* state);

public:
	static ExpressionRef parse(ParsingState* state)
	{
		ExpressionParser parser(state->get_msg());
		return parser.parse(state);
	}
};