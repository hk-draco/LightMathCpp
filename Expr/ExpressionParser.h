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
				msg->add_err(std::to_string(i) + "�������󂯎��$O0�͂���܂���", ope->area());
				break;
			}

			expr->set_arg(arg_num - i, exprs.pop_back());
		}

		push_expr(expr);
	}

public:
	ExpressionParseState(MessageManager* msg) : msg(msg), less_term_num(1) {}

	// ���̒ǉ�
	void add_term(Token* token);

	// �֐��ǉ�(�����̐��͎b��1��)
	void add_func(Token* token)
	{
		token->ope = Operator(token);
		opes.push_back(token);
	}

	// ���Z�q�ǉ�(���Ɉ����̐��͊m�肵�Ă���)
	void add_ope(Token* token)
	{
		opes.push_back(token);
		less_term_num += token->ope.get_arg_num();
		less_term_num--; // add stack
	}

	// ���ʂ̒ǉ�
	void add_bracket(Token* token)
	{
		token->ope = Operator(token);
		opes.push_back(token);
		less_term_num++; // === "("
	}

	// �^�v���p�y�їD��p�̊��ʂ̒ǉ�(�R���}�ɂ���Ĉ������m�肷��)
	void add_tuple_bracket(Token* token)
	{
		token->ope = Operator(token);
		opes.push_back(token);
		less_term_num++; // === "("
	}

	// �ł��V�����֐��̈����̐��𑝂₷
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

	// ���Z�q�X�^�b�N�̐擪���J�����ʂɂȂ�܂Ŏ����쐬����
	void make_expr_to_bracket()
	{
		while (opes) {
			if (opes.back()->ident == CTS("("))
				break;
			else
				make_expr(opes.pop_back());
		}
	}

	// ���Z�q�X�^�b�N�̐擪�̊��ʂ��|�b�v���邩�^�v�����쐬����
	void pop_bracket()
	{
		if (!opes || opes.back()->ident != CTS("(")) {
			msg->add_err("���ʂ������Ă��܂���", exprs.back()->get_area());
			return;
		}
		if (opes.back()->ope.get_arg_num() > 1) {
			opes.back()->ident = CTS("tuple");
			make_expr(opes.back());
		}
		opes.pop_back();
		less_term_num--; // === ")"
	}

	// ���Z�q�X�^�b�N�̐擪����C�ӂ̉��Z�q���D�揇�ʂ̍������Z�q�̑S�Ă̎����쐬
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

	// �S�Ă̎����쐬����
	void make_expr_all()
	{
		for (auto ope : opes.range().reversed()) {
			if (ope->ident == CTS("("))
				msg->add_err("���ʂ������Ă��܂���", ope->area());
			make_expr(ope);
		}
	}

	// �����I�����Ă��邩�ǂ���
	bool is_ended()
	{
		// �I������: �X�^�b�N�̉��Z�q�̈����̍��v - �X�^�b�N�̉��Z�q�̐� + 1 = �o�̓X�^�b�N�̍��̐�
		// less_term_num = �X�^�b�N�̉��Z�q�̈����̍��v - �X�^�b�N�̉��Z�q�̐� - �o�̓X�^�b�N�̍��̐� + 1
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