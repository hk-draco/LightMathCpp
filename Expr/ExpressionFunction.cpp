#include "ExpressionFunction.h"

ExpressionRef ExpressionFunction::clone_latest() const
{
	return apply_args([](ExpressionRef arg) {return arg->clone_latest(); });
}

bool ExpressionFunction::equals(const ExpressionRef trg) const
{
	auto func = [](ExpressionRef arg, ExpressionRef trg_arg) {return arg->equals(trg_arg); };
	return for_each_args(trg, func);
}

bool ExpressionFunction::unify(const ExpressionRef trg, AssignTable* asg) const
{
	auto func = [=](ExpressionRef arg, ExpressionRef trg_arg) {return arg->unify(trg_arg, asg); };
	return for_each_args(trg, func);
}

ExpressionRef ExpressionFunction::assign(const AssignTable& asg) const
{
	return apply_args([&](ExpressionRef arg) {return arg->assign(asg); });
}

CompareResult ExpressionFunction::compare(const ExpressionRef trg) const
{
	if (auto func = trg.cast<ExpressionFunction>()) {
		if (auto res = var->get_name().compare(func->var->get_name()); res != CompareResult::Equal)
			return res;
		if (auto res = ::compare(func->args.size(), args.size()); res != CompareResult::Equal)
			return res;
		for (auto[arg, trg_arg] : Zip(args.range(), func->args.range())) {
			if (auto res = arg->compare(trg_arg); res != CompareResult::Equal)
				return res;
		}
		return CompareResult::Equal;
	}
	return CompareResult::Larger;
}

const ExpressionTypeFunction* ExpressionFunction::complete_func(Scope* scope, MessageManager* msg)
{
	var = scope->get_var(*token);
	if (!var) {
		msg->add_err("識別子$Iは宣言されていません", token->area())->add(token->str);
		return nullptr;
	}

	auto var_type = var->get_type();
	auto type_func = dynamic_cast<const ExpressionTypeFunction*>(type);
	if (!type_func) {
		msg->add_err("$Iは関数ではありません", token->area())->add(token->str);
		return nullptr;
	}

	type = type_func->get_return_type();
	return type_func;
}

bool ExpressionFunction::complete(Scope* scope, MessageManager* msg)
{
	auto type_func = complete_func(scope, msg);
	if (!type_func)
		return false;

	auto arg_types = type_func->get_arg_type();
	if (args.size() != arg_types.size()) {
		msg->add_err("引数の数が異なります", token->area())->add(token->str);
		return false;
	}

	bool successful = true;
	for (auto[arg, arg_type] : Zip(args.range(), arg_types.range())) {
		if (!arg->complete(scope, msg)) {
			successful = false;
			continue;
		}

		if (!arg_type->complete(scope, msg, arg)) {
			successful = false;
			msg->add_err("型が異なります", token->area())->add(token->str);
		}
	}
	
	return successful;
}

bool ExpressionFunctionAC::complete(Scope* scope, MessageManager* msg)
{
	auto type_func = complete_func(scope, msg);
	if (!type_func)
		return false;

	auto arg_type = type_func->get_arg_type()[0];

	bool successful = true;
	for (auto arg : args.range()) {
		if (!arg->complete(scope, msg)) {
			successful = false;
			continue;
		}

		if (!arg_type->complete(scope, msg, arg)) {

		}
	}
}

bool ExpressionFunctionC::unify(const ExpressionRef trg, AssignTable* asg) const
{
	auto func_trg = trg.cast<ExpressionFunctionC>();
	if (!func_trg || var != func_trg->var)
		return false;
	int asg_size = asg->size();
	if (args[0]->unify(func_trg->args[0], asg)
		&& args[1]->unify(func_trg->args[1], asg)) {
		return true;
	}
	asg->resize(asg_size);
	if (args[1]->unify(func_trg->args[0], asg)
		&& args[0]->unify(func_trg->args[1], asg)) {
		return true;
	}
	asg->resize(asg_size);
	return false;
}


// 順不同でptnsをそれぞれtrgsのどれかに単一化する(各式は一度しか単一化しない)
// rest_args: 単一化しなかったtrgs(util_nextを用いたリスト)
bool match_in(RefArray<ExpressionRef> trgs, RefArray<ExpressionRef> ptns, AssignTable* asg)
{
	if (!ptns)
		return true;

	for (auto trg : trgs.range().indexed()) {
		if (!trg.value)
			continue;
		if (ptns.front()->unify(trg.value, asg)) {
			// trgs: trgと等しいものを削除
			// ptns: 最初のものを削除 
			auto size = asg->size();
			trgs[trg.index] = nullptr;
			if (match_in(trgs, ptns.next(), asg))
				return true;
			asg->resize(size);
			trgs[trg.index] = trg.value; // 削除したのを戻す
		}
	}

	return false;
}

bool ExpressionFunctionAC::unify(const ExpressionRef trg, AssignTable* asg) const
{
	auto func_trg = trg.cast<ExpressionFunctionAC>();
	if (!func_trg || var != func_trg->var)
		return false;

	if (FixedArray<ExpressionRef> new_trgs(func_trg->args); match_in(new_trgs, args, asg)) {
		auto rest = new_trgs.null_removed();
		if (bind_var >= 0) {
			if (rest.size() == 1) {
				(*asg)[bind_var] = rest.front();
			} else if (rest.size() > 1) {
				auto expr = new(rest.size()) ExpressionFunctionAC(var, rest.size());
				(*asg)[bind_var] = expr;
			}
			return true;
		} else {
			return !rest;
		}
	}
	return false;
}

ExpressionRef ExpressionFunctionAC::assign(const AssignTable& asg) const
{
	FixedArray<ExpressionRef> list;
	auto add_arg = [&](const ExpressionRef arg) {
		if (!arg)
			return;
		auto assigned = arg->assign(asg);
		auto assigned_ac = dynamic_cast<ExpressionFunctionAC*>(assigned);
		if (assigned_ac && assigned_ac->var == var) {
			for (auto arg_arg : assigned_ac->args.range())
				list.push_back(arg_arg);
		} else {
			list.push_back(assigned);
		}
	};

	for (auto arg : args.range())
		add_arg(arg);
	if (bind_var >= 0)
		add_arg(asg[bind_var]);

	if (!list)
		THROW_EXCEPTION;
	else if (list.size() == 1)
		return list.front();
	else
		return new(list.size()) ExpressionFunctionAC(var, RefArray<ExpressionRef>(list));
}

/*
bool ExpressionFunction::assigned_equals(const AssignTable& asg, const ExpressionRef trg) const
{
	if (type == ExpressionType::Step)
		return args.back()->equals(trg);

	if (type == ExpressionType::Variable) {
		auto expr = asg[var_idx];
		if (expr->type == ExpressionType::Variable)
			return expr->ident != trg->ident;
		else
			expr->assigned_equals(asg, trg);
	}

	if (type != trg->type)
		return false;

	if (ident != trg->ident)
		return false;
	for (auto[arg, trg_arg] : Zip(args.range(), trg->args.range())) {
		if (!arg->assigned_equals(asg, trg_arg))
			return false;
	}
	return true;
}
*/