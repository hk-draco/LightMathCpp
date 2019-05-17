#include "Declaration.h"
#include "ExpressionLambda.h"
#include "ExpressionType.h"

void DeclarationTheorim::run(Scope* scope)
{
	for (auto stm : body.range()) {
		stm->run(&scope, nullptr);
	}

	for (auto stm : body.range()) {
		stm->run(&scope, &prop);
	}

	RunningState state = {&scope, glb->get_msg(), };
}

void DeclarationAxiom::run(Scope* scope)
{
	for (auto stm : body.range()) {
		stm->run(&scope, nullptr);
	}
}

void DeclarationDefine::run(Scope* scope)
{
	if (var_dec) {
		MemberArray<TypedVariable> args;
		body = new(var_dec->typings.range().count()) ExpressionLambda(var_dec, body);
	}
}

void DeclarationUndefinedTerm::run(Scope* scope)
{
	eval_type->complete(scope, msg);
}

void DeclarationType::run(Scope* scope)
{
	// type RVec(n: N) extends NSub(1, n)->R
	// RVec = (n: N)->TypeObject{RVec(n: N)}

	auto type = new Type();
	int arg_num = var_dec->typings.range().count();
	auto ret_type = new ExpressionTypeAtom(type, arg_num);
	for (auto[typing, idx] : var_dec->typings.range().indexed()) {
		ret_type->set_arg(idx, new ExpressionAtom(typing->ident)))
	}

	auto cstr = new ExpressionTypeFunction(base, arg_num);
	for (auto[typing, idx] : var_dec->typings.range().indexed()) {
		cstr->set_arg_type(idx, typing->type);
	}
}

/*
void DeclarationProp::add_prop(Scope* scope)
{
	auto block = dec->body.get();
	if (!block) {
		msg->add_err("命題がありません", dec->area);
		return nullptr;
	}
	auto prop = get_expr_from_stms(block);
	if (!prop)
		return nullptr;

	auto scope = create_scope(dec->var_dec);

	auto before = prop->get_args().front();
	if (!before) {
		msg->add_err("関数である必要があります", dec->area);
		return nullptr;
	}
	auto after = prop->get_args().get_next().front();
	if (!after) {
		msg->add_err("1引数関数は使用できません", dec->area);
		return nullptr;
	}
	auto b_res = make_expr(before, scope);
	bool a_res = make_expr(after, scope);
	if (!b_res || !a_res)
		return nullptr;

	bool is_imply;
	switch (prop->get_func_name()->ident) {
	case CTS("simpl")
		is_imply = false;
		break;
	case CTS("implies"):
		is_imply = true;
		break;
	case CTS("equals"):
		add_equiv_rule(before, after, prop->get_area());
		return nullptr;
	default:
		msg->add_err("$Fは使用できません", prop->get_head()->area())->add(prop->get_head()->str);
		return nullptr;
	}

	if (!prop->get_args().front()->is_func())
		return nullptr; // TODO

	if (!is_imply) {
		if (Expression::make_ac_rule(before, prop->get_args().get_next_entity_ptr(), scope.get_var_num()))
			scope.add_rest_vars();
		after = prop->get_args().get_next().front();
	}

	auto rule = std::make_unique<RewritingRule>(std::move(scope), before, after, is_imply, prop->get_area());
	if (!check_simpl(rule.get()))
		return nullptr;

	auto res = rule.get();
	rule->get_before()->get_obj()->add_rewriting_rule(std::move(rule));
	return res;
}

bool check_simpl(const RewritingRule* rule)
{
	auto left_func = rule->get_before()->get_obj();
	if (!rule->get_after()->is_func()) {
		auto cmp = glb->get_simplicity()->check_simplity(rule);
		if (cmp == SimplityCompare::Equal) {
			msg->add_err("両辺の簡略性が等価です", rule->get_area());
			return false;
		} else if (cmp == SimplityCompare::NG) {
			return false;
		}
	}
	return true;
}

void add_equiv_rule(ExpressionRef before, ExpressionRef after, Area area)
{
	if (is_commutative_law(before, after)) {
		msg->add_msg("$Fの交換律を認識しました", area)->add(before->get_func_name()->str);
		before->get_obj()->enable_commutative();
	} else if (is_associative_law(before, after)) {
		msg->add_msg("$Fの結合律を認識しました", area)->add(before->get_func_name()->str);
		before->get_obj()->enable_associative();
	} else {
		msg->add_err("交換律でも結合律でもありません", area);
	}
}

// 命題(公理・定理)の追加
GlobalEntity* add_prop(Declaration* dec)
{
	
}

GlobalEntity* add_obj(const Declaration* dec, bool has_def)
{
	auto scope = create_scope(dec->var_dec);
	if (!make_type_expr(dec->eval_type.get(), scope))
		return nullptr;

	ExpressionRef def = nullptr;
	if (has_def) {
		def = get_expr_from_stms(dec->body.get());
		if (make_expr(def, scope)) {
			if (!dec->eval_type->equals(def->eval()))
				msg->add_err("戻り値が$Tではありません", def->get_area())->add(dec->eval_type->to_str());
		} else {
			return nullptr;
		}
	} else {
		if (dec->body)
			msg->add_err("本体は不要です", dec->body->stms.front()->area);
	}

	auto obj = std::make_unique<Object>(dec->name, std::move(scope), dec->eval_type.get(), def);

	return glb->add_obj(std::move(obj));
}

GlobalEntity* add_type(const Declaration* dec)
{
	auto scope = create_scope(dec->var_dec);

	ExpressionRef cond;
	if (dec->body) {
		cond = get_expr_from_stms(dec->body.get());
		if (cond && !make_expr(cond, scope))
			return nullptr;
	}

	auto base = dec->eval_type.get();
	if (base && !make_expr(base, scope))
		return nullptr;

	auto type = std::make_unique<Type>(dec->name->str, std::move(scope), base, cond);
	return glb->add_type(std::move(type));
}*/