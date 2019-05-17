#include "Expression.h"
#include "ExpressionStep.h"
#include "ExpressionFunction.h"
#include "ExpressionTyping.h"

void ExpressionRef::rewrite(ExpressionRef new_expr, RewritingInfo* rewriting)
{
	if (ptr)
		ptr = new ExpressionStep(new_expr, ptr, rewriting);
	else
		ptr = new_expr.ptr;
}

struct ExpressionRewriter
{
	ExpressionFunction** trg;
	int simplity;

	ExpressionRewriter(ExpressionFunction** trg)
		: trg(trg), simplity((*trg)->get_obj()->get_simplity())
	{}
};

void ExpressionRef::lookup_rewriters(FixedArray<ExpressionRewriter>* rewriters)
{
	auto latest = get_latest_step();
	auto expr = *latest;
	if (auto func = dynamic_cast<ExpressionFunction*>(expr.get())) {
		rewriters->push_back(ExpressionRewriter((ExpressionFunction**)latest));
		for (auto& arg : func->get_args().range())
			ExpressionRef(arg).lookup_rewriters(rewriters);
	}
}

bool ExpressionRef::simplify()
{
	for (int i = 0; i < 1000; i++) {
		FixedArray<ExpressionRewriter> rewriters;
		lookup_rewriters(&rewriters);
		auto cmp = [](const ExpressionRewriter& a, const ExpressionRewriter& b) { return a.simplity < b.simplity; };
		std::sort(rewriters.range().begin(), rewriters.range().end(), cmp);
		for (auto rewriter : rewriters.range()) {
			if ((*rewriter.trg)->get_obj()->get_equiv_rules().rewrite(&ptr, RewritingType::Auto))
				continue;
		}
		return true;
	}
	return false;
}

bool ExpressionRef::derivate_whole(const ExpressionRef goal)
{
	if (auto func = cast<ExpressionFunction>()) {
		if (func->get_obj()->get_imp_rules().rewrite(this, goal, RewritingType::User))
			return true;
	}
	return false;
}

bool ExpressionRef::derivate(const ExpressionRef goal)
{
	auto diff = difference(goal);
	do {
		if (diff->expr->derivate_whole(diff->trg_expr))
			return true;
	} while (diff = diff->parent);
	return false;
}

bool ExpressionRef::transform(const ExpressionRef goal)
{
	if (ptr->equals(goal))
		return;
	auto goal_clone = goal->clone_latest();
	if (simplify() && goal_clone.simplify()) {
		if (ptr->equals(goal_clone)) {
			merge_steps(goal_clone);
			return true;
		}
	}
	return false;
}

bool ExpressionRef::unfold(const ExpressionRef goal, Scope* scope)
{
	auto diff = difference(goal);
	auto func = diff->expr->cast<ExpressionFunction>();
	if (!func)
		return false;
	auto obj = func->get_obj();
	auto unfold = obj->get_def();
	AssignTable asg(obj->get_scope()->get_var_num());
	if (unfold->unify(diff->trg_expr, &asg)) {
		rewrite(unfold->assign(asg), new RewritingInfo(RewritingType::Unfold, obj, asg));
		return true;
	}
	for (auto expr : asg.range())
		scope->add_local();
	return false;
}

bool ExpressionRef::typing_unfold(const ExpressionRef goal)
{
	auto typing = cast<ExpressionTyping>();

}

bool ExpressionRef::extract_forall(const ExpressionRef goal)
{
	auto atom = cast<ExpressionAtom>();
	if (atom->get_var()->get_qtf() != Quantifier::ForAll)
		return false;
	if (!goal->evaluate()->equals(atom->evaulate()))
		return false;
	rewrite(goal, new RewritingInfo(RewritingType::ExtractForall, obj));
	return true;
}

bool ExpressionRef::app_exists(const ExpressionRef goal)
{
	auto goal_atom = goal.cast<ExpressionAtom>();
	if (goal_atom->get_var()->get_qtf() != Quantifier::Exists)
		return false;
	if (!goal->evaluate()->equals(atom->evaulate()))
		return false;
	rewrite(goal, new RewritingInfo(RewritingType::ExtractForall, obj));
	return true;
}

ExpressionRef* ExpressionRef::get_latest_step()
{
	if (auto step = dynamic_cast<ExpressionStep*>(ptr))
		return step->get_latest_step();
	return this;
}

struct ExpressionJunction
{
	ExpressionRef expr;
	RewritingInfo* latter;
	RewritingInfo* former;

	ExpressionJunction(std::nullptr_t) : expr(nullptr) {}

	ExpressionJunction(ExpressionRewritingPair latter_pair, ExpressionRewritingPair former_pair)
	{
		expr = latter_pair.expr;
		expr.merge_steps(former_pair.expr);
		latter = latter_pair.rewriting;
		former = former_pair.rewriting;
	}

	operator bool() const
	{
		return expr;
	}
};

bool ExpressionRef::merge_steps(ExpressionRef trg, ExpressionJunction junction)
{
	// merge([A, g([Q, P], E), f([X, Y], Z)], [C, g(P, [F, E]), f([X, Y], Z)])
	// merge([A, g([Q, P], E)], [C, g(P, [F, E])], merge(f([X, Y], Z), f([X, Y], Z)))
	// merge([A, g([Q, P], E)], [C, g(P, [F, E])], f(merge([X, Y], [X, Y]), merge(Z, Z)))
	// merge([A, g([Q, P], E)], [C, g(P, [F, E])], f([X, Y], Z))
	// merge([A], [C], merge(g([Q, P], E), g(P, [F, E])))
	// merge([A], [C], g(merge([Q, P], P), merge(E, [F, E])))
	// merge([A], [C], g([Q, P], [F, E]))
	// [A, g([Q, P], [F, E]), C]

	// merge(A 1 B 2 C 3 D, P 4 Q 5 C 6 D)
	// merge(A 1 B 2 C, P 4 Q 5 C, merge(3 D, 6 D))
	// merge(A 1 B, P 4 Q, merge(2 C, 5 C))
	// merge(A 1 B, P 4 Q, 2 C 5))
	// A 1 B 2 C 5 P 4 Q

	if (auto trg_step = trg.cast<ExpressionStep>()) {
		for (auto[step, rewriting] : trg_step->step_range())
			merge_steps(step, rewriting);
		return true;
	}

	auto func = cast<ExpressionFunction>();
	auto trg_func = trg.cast<ExpressionFunction>();
	if (func->get_obj() == trg_func->get_obj()) {
		auto range = RefZip(func->get_args().range(), trg_func->get_args().range());
		for (auto[former_arg, latter_arg] : range) {
			former_arg->merge(*latter_arg);
		}
		return true;
	}

	auto step = cast<ExpressionStep>();
	auto trg_step = trg.cast<ExpressionStep>();
	if (step && trg_step && ptr->equals(trg)) {
		junction = ExpressionJunction(pop_step(), trg.pop_step());
		merge_steps(trg, junction);
	} else if (junction) {
		rewrite(junction.expr, junction.former);
		if (trg) {
			if (auto trg_step = trg.cast<ExpressionStep>()) {
				for (auto[step, rewriting] : trg_step->step_range())
					rewrite(step, rewriting ? rewriting : junction.latter);
			} else {
				rewrite(trg, junction.latter);
			}
		}
	} else {
		return false;
	}

	return true;
}

bool ExpressionRef::merge_steps(ExpressionRef trg)
{
	return merge_steps(trg, nullptr);
}

ExpressionDifference* ExpressionRef::difference(const ExpressionRef trg)
{
	auto func = cast<ExpressionFunction>();
	auto trg_func = trg.cast<ExpressionFunction>();

	if (func && trg_func && func->get_obj() == trg_func->get_obj()) {
		int diff_num = 0;
		ExpressionDifference* diff_one;
		for (auto[arg, trg_arg] : RefZip(func->get_args().range(), trg_func->get_args().range())) {
			auto diff = arg->difference(trg_arg);
			if (diff.expr) {
				diff_one = diff;
				diff_num++;
			}
		}
		if (diff_num == 0)
			return nullptr;
		else if (diff_num == 1)
			return new ExpressionDifference(this, trg, diff_one);
		else if(cast<ExpressionFunctionAC>()) {
			FixedArray<Expression*> args(func->get_args());
			FixedArray<Expression*> trg_args(trg_func->get_args());
			for (auto[arg, idx] : args.range().indexed()) {
				for (auto[trg_arg, trg_idx] : trg_args.range().indexed()) {
					if (!trg_arg)
						continue;
					if (arg->equals(trg_arg)) {
						trg_args[trg_idx] = nullptr;
						args[idx] = nullptr;
						break;
					}
				}
			}


		}

	} else if (!func && !trg_func) {
		auto atom = cast<ExpressionAtom>();
		auto trg_atom = trg.cast<ExpressionAtom>();
		if (atom && trg_atom) {
			if (atom->get_obj() == trg_atom->get_obj())
				return nullptr;
		}
	}

	return new ExpressionDifference(this, trg);
}

ExpressionRewritingPair ExpressionRef::pop_step()
{
	assert(cast<ExpressionStep>());
	auto step = (ExpressionStep*)ptr;
	*this = step->get_older();
	return { step->get_latest_step(), step->get_rewriting() };
}