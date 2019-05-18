#pragma once

struct LabeledBlock
{
	Token* keyword;
	Iteratable<Statement> stms;

	LabeledBlock(Token* keyword, Iteratable<Statement> stms)
		: keyword(keyword), stms(stms)
	{}

	LabeledBlock(Iteratable<Statement> stms)
		: stms(stms)
	{}

	ExpressionRef get_stms_expr(MessageManager* msg)
	{
		if (!stms) {
			msg->add_err("��������܂���", block->label->area);
			return nullptr;
		}

		if (stms.back()->type != StatementType::Expression) {
			msg->add_err("�Ō�̃X�e�[�g�����g�͎��ł���K�v������܂�", block->stms.back()->area);
			return nullptr;
		}

		return stms.back()->arg.get();
	}

	StringView get_str_from_expr(ExpressionRef expr)
	{
		if (expr->get_head()->ann != Annotator::String) {
			msg->add_err("������ɕ]���ł��܂���", expr->get_head()->area());
			return nullptr;
		}
		return expr->get_head()->str;
	}

	StringView get_stms_str()
	{
		if (auto expr = get_stms_expr())
			return get_str_from_expr(expr);
		return nullptr;
	}

	StringView get_arg_str(MessageManager* msg)
	{
		if (auto expr = label->arg.)
			return get_str_from_expr(expr);
		msg->add_err("����������܂���", label->area);
		return StringView();
	}
};
