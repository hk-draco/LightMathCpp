#include "Type.h"

bool TypingRule::match(const ExpressionRef trg, RefArray<ExpressionRef> args) const
{
	if (AssignTable asg; expr->unify(trg, &asg)) {
		for (auto[type_arg, idx] : type_args.range().indexed()) {
			args[idx] = type_arg->assign(asg);
		}
		return true;
	}
	return false;
}

bool TypingRuleSet::match(const ExpressionRef trg, RefArray<ExpressionRef> args) const
{
	for (auto rule : rules.range()) {
		if (rule.match(trg, args))
			return true;
	}
	return false;
}

bool Type::cast(const ExpressionRef trg, RefArray<ExpressionRef> args)
{
	if (typing.match(trg, args))
		return true;
	return false;
}

// type NVec(n) extends RVec(n)
// from = NVec(3): type, to = RVec(3): type => true
bool down_castable(const ExpressionRef from, const ExpressionRef to)
{
	auto from_type = from->get_type();
	auto base = from_type->get_base();
	if (!base)
		return false;

	// 型名チェック
	if (base->get_type() != to->get_type())
		return false;

	// 引数チェック
	AssignTable asg(from_type->get_scope()->get_arg_num());
	from_type->get_scope()->unify_args(&asg, from->get_args());
	if (base->assigned_equals(asg, to))
		return true;

	return false;
}

// type NotZero(T: type) extends T { this != 0; }
// expr = Pi: R, type = NotZero(R): type => true
// type RiemannIntegrable(a, b: R) extends R->R { IsRiemannIntegrable(this, a, b); }
// expr = sin: R->R; type = RiemannIntegrable(1, 2): type => true
bool up_castable(const ExpressionRef from_expr, const ExpressionRef to)
{
	auto to_type = to->get_type();
	auto asg = to_type->get_scope()->unify_args(to->get_args());
	auto base = to_type->get_base();
	if (!base)
		return false;

	// 目標型の基底型と式の型が一致するか確認
	if (!base->assigned_equals(asg, from_expr->eval()))
		return false;
}

bool evaluatable(const ExpressionRef expr, const ExpressionRef type)
{
	auto from = expr->eval();
	if (from->equals(type))
		return true;

	if (down_castable(from, type))
		return true;

	if (up_castable(expr, type))
		return true;
	return false;
}