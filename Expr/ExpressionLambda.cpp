#include "ExpressionLambda.h"

ExpressionRef ExpressionLambda::clone_latest() const
{
	auto expr = new(args.size()) ExpressionLambda(obj, args.size());
	for (auto[arg, idx] : args.range().indexed())
		expr->args[idx] = arg->clone_latest();
	return expr;
}

bool ExpressionLambda::unify(const ExpressionRef trg, AssignTable* asg) const
{
	if (trg->type != ExpressionType::Lambda || args.size() != trg->args.size())
		return false;
	AssignTable local_asg(asg.size());
	local_asg.marge(asg);
	unify_lambda_args(&local_asg, trg);
	return args.back()->unify(trg->args.back(), local_asg, out_asg);
}

ExpressionRef ExpressionLambda::assign(const AssignTable& asg) const
{
	auto expr = Expression::create(head, args.size());
	for (auto[arg, index] : args.range().cut_back().indexed())
		expr->set_arg(index, arg);
	expr->set_arg(args.size() - 1, args.back()->assign(asg));
	expr->make_lambda();
	return expr;
}

bool ExpressionLambda::unify_lambda_args(AssignTable* asg, const ExpressionRef trg) const
{
	for (auto arg : Zip(args.range(), trg->args.range()).cut_back()) {
		if (arg.first->eval_type != arg.second->eval_type)
			return false;
		(*asg)[arg.first->var_idx] = arg.second;
	}
	return true;
}

bool Expression::equals(const ExpressionRef trg) const
{
	AssignTable asg(AssignTable::MAX_SIZE);
	unify_lambda_args(&asg, trg);
	return args.back()->assigned_equals(asg, trg->args.back());
}