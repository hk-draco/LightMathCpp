#include "Expression.h"




void Expression::replace(std::intptr_t ident, ExpressionRef expr)
{
	switch (type) {
	case ExpressionType::Step:
	case ExpressionType::Lambda:
		return args.back()->replace(ident, std::move(expr));

	case ExpressionType::Variable:
		if (this->ident == ident) {
			assert(false);//TODO
			//*this = std::move(*expr->clone().release());
		}

	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:
		
	default:
		THROW_EXCEPTION;
	}
}

ExpressionRef Expression::replace_once(std::intptr_t ident, const ExpressionRef expr)
{
	switch (type) {
	case ExpressionType::Step:
	case ExpressionType::Lambda:
		return args.back()->replace_once(ident, expr);

	case ExpressionType::Constant:
		return nullptr;

	case ExpressionType::Variable:
		if (this->ident == ident) {
			assert(false);
			// TODO
			//*this = std::move(*expr->clone().release());
			return this;
		}
		return nullptr;

	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:
	{
		ExpressionRef replaced = nullptr;
		for (auto arg : args.range()) {
			if (replaced) {
				if (arg->contains(ident))
					return nullptr;
			} else {
				replaced = arg->replace_once(ident, expr);
			}
		}
		return replaced;
	}
	default:
		THROW_EXCEPTION;
		return false;
	}
}

ExpressionRef Expression::find(const ExpressionRef trg, const ExpressionRef trg_child)
{
	if (trg == trg_child)
		return this;

	if (trg->type == ExpressionType::Step)
		trg = trg->args.back();
	if (type == ExpressionType::Step)
		return args.back()->find(trg, trg_child);

	if (type != trg->type)
		return nullptr;

	switch (type) {
	case ExpressionType::Lambda:
		return args.back()->find(trg->args.back(), trg_child);

	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:
		if (ident != trg->ident)
			return nullptr;
		for (auto arg : Zip(args.range(), trg->args.range())) {
			auto res = arg.first->find(arg.second, trg_child);
			if (res)
				return res;
		}
	default:
		THROW_EXCEPTION;
		return false;
	}
}

bool Expression::equals(const ExpressionRef trg) const
{
	if(type == ExpressionType::Step)
		return args.back()->equals(trg);

	if (type != trg->type)
		return false;

	switch (type) {
	case ExpressionType::Constant:
	case ExpressionType::Variable:
		return ident == trg->ident;

	case ExpressionType::Lambda:
	{
		AssignTable asg(AssignTable::MAX_SIZE);
		unify_lambda_args(&asg, trg);
		return args.back()->assigned_equals(asg, trg->args.back());
	}
	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:


	default:
		THROW_EXCEPTION;
		return false;
	}
}

bool Expression::assigned_equals(const AssignTable& asg, const ExpressionRef trg) const
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

	switch (type) {
	case ExpressionType::Constant:
		return ident != trg->ident;
	case ExpressionType::Lambda:
		break;
	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:
		if (ident != trg->ident)
			return false;
		for (auto[arg, trg_arg] : Zip(args.range(), trg->args.range())) {
			if (!arg->assigned_equals(asg, trg_arg))
				return false;
		}
		return true;
	default:
		THROW_EXCEPTION;
		return false;
	}
}

bool Expression::contains(std::intptr_t ident) const
{
	switch (type) {
	case ExpressionType::Step:
	case ExpressionType::Lambda:
		return args.back()->contains(ident);

	case ExpressionType::Constant:
		return false;

	case ExpressionType::Variable:
		return this->ident == ident;

	case ExpressionType::Function:
	case ExpressionType::CFunction:
	case ExpressionType::ACFunction:
		for (auto arg : args.range()) {
			if (arg->contains(ident))
				return true;
		}
		return false;

	default:
		THROW_EXCEPTION;
		return false;
	}
}

ExpressionRef Expression::get_latest_step()
{
	if(type == ExpressionType::Step)
		return args.back()->get_latest_step();
	return this;
}



void Expression::make_func(const ExpressionRef eval_type, std::intptr_t ident, const FunctionProperty& prop)
{
	this->bind_var = -1;
	this->eval_type = eval_type;
	this->ident = ident;
	if (!prop.is_commutative) {
		type = ExpressionType::Function;
		return;
	}
	if (prop.is_associative) {
		FixedArray<Expression> exprs;
		get_exprs(head->str, args, &exprs);
		args = exprs;
		type = ExpressionType::ACFunction;
	} else {
		type = ExpressionType::CFunction;
	}
}

bool Expression::make_ac_rule(ExpressionRef before, ExpressionRef* after_ent, int bind_var)
{
	if (before->type != ExpressionType::ACFunction)
		return false;

	auto after = *after_ent;
	if (!after->is_func() || after->ident != before->ident) {
		List<Expression> args;
		args.push_back(std::move(*after_ent));
		*after_ent = std::make_unique<Expression>(before->get_head(), std::move(args));
		after = after_ent->get();
		after->make_func(before->eval_type, before->ident, ExpressionType::ACFunction);
	}

	Expression *before_var, *after_var;
	int bind_var_idx;

	before_var = before->find_unique_arg([](ExpressionRef arg) { return arg->is_var(); });
	if(!before_var)
		goto MAKE_VAR;

	bind_var_idx = before_var->var_idx;

	// —Bˆê‚Ì•Ï”‚ª•Ê‚Ì€‚ÉŠÜ‚Ü‚ê‚Ä‚¢‚½‚Æ‚«
	if(before->find_arg([=](ExpressionRef arg) {return arg != before_var && arg->contains(before_var->ident); }))
		goto MAKE_VAR;

	before->args.remove(before_var);
	before->bind_var = bind_var_idx;

	after_var = after->find_unique_arg([=](ExpressionRef arg) { return arg->is_var() && arg->var_idx == bind_var_idx; });
	if (!after_var)
		goto MAKE_VAR;
		
	after->args.remove(after_var);
	after->bind_var = bind_var_idx;
	return false;

MAKE_VAR:
	before->bind_var = bind_var;
	after->bind_var = bind_var;
	return true;
}

void lookup_ac_latest_args(std::intptr_t ident, const ExpressionRef trg, FixedArray<ExpressionRef>* latest_trgs)
{
	for (auto arg : trg->get_args().range()) {
		auto latest_arg = arg->get_latest_step();
		if (latest_arg->get_ident() == ident)
			lookup_ac_latest_args(ident, latest_arg, latest_trgs);
		else
			latest_trgs->push_back(latest_arg);
	}
}

bool Expression::commute_ac(ExpressionRef* trg, ExpressionRef rewrote_arg)
{
	auto src = *trg;
	FixedArray<ExpressionRef> new_trgs;
	lookup_ac_latest_args(src->ident, src, &new_trgs);
	if (src->args.size() == new_trgs.size())
		return false;
	auto new_expr = new (new_trgs.size()) Expression(src->type, src->head, src->ident, new_trgs);
	rewrite(trg, new_expr, new RewritingInfo(RewritingType::CommuteAC));
	return true;
}

void Expression::rewrite(ExpressionRef* trg, ExpressionRef new_expr, RewritingInfo* info)
{
	if (info)
		new_expr->rewriting = info;
	else
		assert(new_expr->rewriting);
	auto step = new (2) Expression();
	step->args[0] = new_expr;
	step->args[1] = *trg;
	*trg = step;
}

ExpressionRef Expression::clone_latest() const
{
	if (type == ExpressionType::Step)
		return args.back()->clone_latest();
	auto expr = new(args.size()) Expression(type, eval_type, ident);
	for (auto[arg, idx] : args.range().indexed())
		expr->args[idx] = arg->clone_latest();
	return expr;
}


std::tuple<Evaluatable*, FunctionProperty, bool> get_obj(Token* token, const Scope& scope)
{
	auto ope = token->ope.get_func_name();
	auto func_name = ope ? ope->str : token->str;

	auto var = scope.get_var(func_name);
	if (var)
		return { (Evaluatable*)var, FunctionProperty(), true };
	auto obj = glb->get_obj(func_name);
	if (obj)
		return { obj, obj->get_prop(), false };
	msg->add_err("$I‚ÍéŒ¾‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ", token->area())->add(func_name);
	return { nullptr, FunctionProperty(), false };
}

bool make_expr(ExpressionRef expr, const Scope& scope)
{
	
}

bool make_type_expr(ExpressionRef expr, const Scope& scope)
{
	auto type = glb->get_type(expr->get_head()->str);
	if (!expr->get_args()) {
		expr->make_type((std::intptr_t)glb->get_type(expr->get_head()->str));

	} if (expr->get_head()->ident == CTS("__func")) {
		auto args = expr->get_args().front();
		auto retn = expr->get_args().next().front();
		expr->make_func_type();
		args->make_type_args();
		retn->make_type_args();
		bool has_error = false;
		for (auto arg : args->get_args().range()) {
			if (!make_type_expr(arg, scope))
				has_error = true;
		}
		if (has_error)
			return false;
		if (!make_type_expr(retn, scope))
			return false;

	} else {
		bool has_error = false;
		for (auto arg : expr->get_args().range()) {
			if (!make_expr(arg, scope))
				has_error = true;
		}
		if (has_error)
			return false;
		expr->make_type((std::intptr_t)glb->get_type(expr->get_head()->str));
	}
	return true;
}

void Expression::complete(const GlobalScope& glb, const Scope& scope)
{
	// æ‚Éˆø”‚ðˆ—‚·‚é
	bool has_error = false;
	for (auto arg : expr->get_args().range()) {
		if (!make_expr(arg, scope))
			has_error = true;
	}
	if (has_error)
		return false;

	auto[obj, prop, is_var] = get_obj(expr->get_head(), scope);
	if (!obj)
		return false;

	if (expr->get_args()) {
		// Ž®Œ`¬
		auto ret_type = eval_func_call(expr, (Object*)obj);
		if (ret_type)
			expr->make_func(ret_type, (std::intptr_t)obj, prop);
	} else {
		if (is_var)
			expr->make_var(obj->evaluate(), (std::intptr_t)obj, ((Variable*)obj)->get_idx());
		else
			expr->make_cst(obj->evaluate(), (std::intptr_t)obj);
	}

	return true;
}



void make_func(const ExpressionRef eval_type, std::intptr_t ident, const FunctionProperty& prop);

void make_cst(const ExpressionRef eval_type, std::intptr_t ident)
{
	type = ExpressionType::Constant;
	this->eval_type = eval_type;
	this->ident = ident;
}

void make_var(const ExpressionRef eval_type, std::intptr_t ident, char var_idx)
{
	type = ExpressionType::Variable;
	this->eval_type = eval_type;
	this->ident = ident;
	this->var_idx = var_idx;
}

void make_type(std::intptr_t ident)
{
	type = ExpressionType::Type;
	this->ident = ident;
}

void make_func_type()
{
	type = ExpressionType::FunctionType;
}

void make_type_args()
{
	type = ExpressionType::TypeArgs;
}

void make_lambda()
{
	type = ExpressionType::Lambda;
	bind_var = args.pop_front().move_entity()->var_idx;
}