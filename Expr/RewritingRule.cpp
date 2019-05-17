#include "RewritingRule.h"
#include "Object.h"
#include "ExpressionFunction.h"

// f(x, y) = f(y, x)
bool RewritingRule::is_commutative_law()
{
	auto before = this->before.cast<ExpressionFunction>();
	auto after = this->after.cast<ExpressionFunction>();
	if (!before || !after)
		return false;

	auto func = before->get_obj();
	if (func->get_scope()->get_arg_num() != 2)
		return false;
	if (func != after->get_obj())
		return false;

	if (!(before->get_args()[0]->is_var() && before->get_args()[1]->is_var()
		&& after->get_args()[0]->is_var() && after->get_args()[1]->is_var()))
		return false;

	return before->get_args()[0]->equals(after->get_args()[1])
		&& after->get_args()[0]->equals(before->get_args()[1]);
}

// f(f(x, y), z) = f(x, f(y, z))
bool RewritingRule::is_associative_law()
{
	auto before = this->before.cast<ExpressionFunction>();
	auto after = this->after.cast<ExpressionFunction>();
	if (!before || !after)
		return false;

	auto func = before->get_obj();
	if (func->get_scope()->get_arg_num() != 2)
		return false;
	if (func != after->get_obj())
		return false;

	auto before_arg0 = before->get_args()[0];
	auto before_arg1 = before->get_args()[1];
	auto after_arg0 = after->get_args()[0];
	auto after_arg1 = after->get_args()[1];

	if ((before_arg0->is_func() && after_arg1->is_func() && func == before_arg0->get_obj() && func == after_arg1->get_obj())
		|| (before_arg1->is_func() && after_arg0->is_func() && func == before_arg1->get_obj() && func == after_arg0->get_obj())) {
		return true;//before->equals(after);
	}
	return false;
}

bool RewritingRule::rewrite(ExpressionRef* trg, RewritingType type)
{
	AssignTable asg(get_scope()->get_var_num());
	if (before->unify(*trg, &asg)) {
		auto new_expr = after->assign(asg);
		Expression::rewrite(trg, new_expr, new RewritingInfo(type, this, asg));
		return true;
	}
	return false;
}

bool RewritingRule::rewrite(ExpressionRef* trg, ExpressionRef goal, RewritingType type)
{
	AssignTable asg(get_scope()->get_var_num());
	if (before->unify(*trg, &asg) && after->unify(goal, &asg)) {
		auto new_expr = after->assign(asg);
		Expression::rewrite(trg, new_expr, new RewritingInfo(type, this, asg));
		return;
	}
}

bool RewritingRuleSet::rewrite(ExpressionRef* trg, RewritingType type)
{
	for (auto rule : rules.range()) {
		if (rule->rewrite(trg, type))
			return true;
	}
	return false;
}

bool RewritingRuleSet::rewrite(ExpressionRef* trg, ExpressionRef goal, RewritingType type)
{
	for (auto rule : rules.range()) {
		if (rule->rewrite(trg, goal, type))
			return true;
	}
	return false;
}