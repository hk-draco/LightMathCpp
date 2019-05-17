#include "Statement.h"

void StatementStep::run(RunningState* state)
{
	expr->complete(state->scope, state->msg);
	if (!state->trg->transform(expr)) {
		state->trg->rewrite(expr, new RewritingInfo(RewritingType::ErrorAdmitted));
	}
}

void StatementImpl::run(RunningState* state)
{
	expr->complete(state->scope, state->msg);
	if (!state->trg->derivate(expr)) {
		state->trg->rewrite(expr, new RewritingInfo(RewritingType::ErrorAdmitted));
	}
}

void StatementUnfold::run(RunningState* state)
{

}

struct WhereInserter
{
public:
	ExpressionRef inserted;
	const ExpressionRef insert_place;

	WhereInserter(ExpressionRef expr, const ExpressionRef where_expr) : inserted(expr), insert_place(expr)
	{
		if (where_expr) {
			inserted = where_expr;
			insert_place = inserted->replace_once(0, inserted);
		}
	}
};

void StatementWhere::run(RunningState* state)
{
	run_proof_statement(trg, wit.inserted, stm->stms.front(), scope);
	auto new_trg = trg->find(wit.inserted, wit.insert_place);
	if (!new_trg) {
		msg->add_err("•s–¾‚ÈƒGƒ‰[‚Å‚·", stm->area);
		return;
	}
	for (auto child_stm : stm->stms.next().range())
		run_proof_statement(new_trg, nullptr, child_stm, scope);
	break;
}

void StatementTarget::run(RunningState* state)
{
	
}

void StatementFork::run(RunningState* state)
{
	auto expr = new Expression(head, stms.range().count());
	for (auto[stm, idx] : stms.range().indexed()) {
		auto new_trg = (*trg)->clone_latest();
		stm->run(scope, &new_trg);
		expr->set_arg(idx, new_trg);
	}
	trg->rewrite(expr);
}