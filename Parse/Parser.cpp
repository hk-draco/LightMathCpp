#include "Parser.h"
#include "ExpressionParser.h"

void Parser::parse_idents(FixedArray<Token*>* out)
{
	do {
		if (auto ident = state.read_ident()) {
			out->push_back(ident);
		} else {
			state.make_error_curr("識別子が必要です");
			break; // 途中までの変数は利用する ex) "forall a, : R"は"forall a: R"と解釈
		}
	} while (state.read_token(CTS(",")));
}

ListBuilder<VariableTyping> Parser::parse_variable_typings_per_type(bool has_ref_vars)
{
	ListBuilder<VariableTyping> typings;
	do {
		auto token = state.read_ident();
		if (!token) {
			state.make_error_curr("識別子が必要です");
			break; // 途中までの変数は利用する ex) "forall a, : R"は"forall a: R"と解釈
		}

		FixedArray<Token*> ref_vars;
		if (state.maybe_token(CTS("["))) {
			parse_idents(&ref_vars);
			if (state.expect_token(CTS("]"), CTS("|")))
				break;
			if (!has_ref_vars)
				state.add_error("全称化には不要です");
		} else if (has_ref_vars) {
			state.add_error("存在化には、それをスコーレム化した際にマッピングする全称化された変数のリストが必要です");
		}

		typings.push_back(new(ref_vars.size()) EntityList(VariableTyping(token, ref_vars)));
	} while (state.read_token(CTS(",")));
	return typings;
}

ListBuilder<VariableTyping> Parser::parse_variable_typings(Quantifier qtf)
{
	auto idents = parse_variable_typings_per_type(qtf == Quantifier::Exists);
	if (!idents)
		return nullptr;

	if (!state.expect_token(CTS(":"), CTS("|")))
		return nullptr;

	auto type = parse_expression();
	if (!type)
		return nullptr;

	for (auto ident : idents.range())
		ident->type = type;

	return idents;
}

// forall eps: posReal | f: R->R | n: N;
// exists n_0[eps]: N
VariableDeclaration* Parser::parse_variable_declaration(Quantifier qtf)
{
	ListBuilder<VariableTyping> typings;
	do {
		if (auto typing = parse_variable_typings(qtf)) {
			typings.push_back_range(typing);
		} else {
			state.make_error_curr("変数の型付けがありません");
			break; // 途中までの変数は利用する ex) "forall hoge::: | f: R->R"は"forall f: R->R"と解釈
		}
		
	} while (state.read_token(CTS("|")));

	return new VariableDeclaration(qtf, typings);
}

Iteratable<Statement> Parser::parse_statements()
{
	ListBuilder<Statement> stms;
	while (!state.is_ended() && state.token()->ident != CTS("}")) {
		if (state.maybe_token(Annotator::EndLine))
			continue;
		if (auto stm = parse_statement())
			stms.push_back(stm);
		state.expect_end_line();
	}
	return std::move(stms);
}

Iteratable<Statement> parse_statement_ver_dec(Area area, Quantifier qtf)
{
	return Iteratable(Statement(area, state.parse_var_dec(qtf)));
}

TargetType to_target_type(Token* token)
{
	switch (token->ident) {
	case CTS("left"):
		return TargetType::Left;
	case CTS("right"):
		return TargetType::Right;
	case CTS("all"):
		return TargetType::All;
	case CTS("context"):
		return TargetType::Context;
	}
}

Iteratable<Statement> Parser::parse_statement()
{
	switch (state.token()->ident) {
	case CTS("forall"):
		state.step_token();
		return parse_var_dec_statement(Quantifier::ForAll);
	case CTS("exists"):
		state.step_token();
		return parse_var_dec_statement(Quantifier::Exists);
	case CTS("target"):
		state.step_token();
		if (auto token = state.read_ident()) {
			auto type = to_target_type(token);
			return Iteratable(StatementTarget(type)).cast<Statement>();
		}
		return nullptr;
	case CTS("step"):
		state.step_token();
		return parse_expr_statement<StatementStep>();
	case CTS("impl"):
		state.step_token();
		return parse_expr_statement<StatementImpl>();
	case CTS("unfold"):
		state.step_token();
		return parse_expr_statement<StatementUnfold>();
	case CTS("fork"):
		state.step_token();
		return parse_block_statement<StatementFork>();
	case CTS("where"):
		state.step_token();
		return parse_expr_block_statement<StatementWhere>();
	case CTS("when"):
		state.step_token();
		return parse_expr_block_statement<StatementWhen>();
	case CTS("name"):
	case CTS("desc"):
	case CTS("proof"):
	case CTS("latex"):
		return nullptr;
	default:
		return parse_expr_statement<StatementExpression>();
	}
}

Iteratable<Statement> Parser::parse_statement_block()
{
	state.skip_end_line();
	if (state.expect_token(CTS("{"), CTS("}"))) {
		auto stms = parse_statements();
		state.expect_token(CTS("}"), CTS("}"));
		return stms;
	} else {
		state.skip_end_line();
		if (auto stm = parse_statement())
			return stm;
		else
			return nullptr;
	}
}

bool Parser::parse_declaration_operator()
{
	// 演算子
	auto token = state.read_token();
	if (!token)
		return false;

	// "("
	if (!state.expect_token(CTS("("), CTS(";")))
		return false;

	// 引数の数
	auto arg_num = state.read_token(Annotator::Integer);
	if (!arg_num)
		return false;

	// ","
	if (!state.expect_token(CTS(","), CTS(";")))
		return false;

	// 優先順位
	auto level = state.read_token(Annotator::Integer);
	if (!level)
		return false;

	// 結合性
	bool left_assoc = true;
	if (state.maybe_token(CTS(","))) {
		auto assoc = state.read_token();
		if (assoc->ident == CTS("left"))
			left_assoc = true;
		else if (assoc->ident == CTS("right"))
			left_assoc = false;
		else
			msg.add_err("結合性を指定してください", assoc->area());
	}

	// ")"
	if (!state.expect_token(CTS(")"), CTS(";")))
		return false;

	// "=>"
	if (!state.expect_token(CTS("=>"), CTS(";")))
		return false;

	// 関数名
	auto func = state.read_token();

	state.expect_end_line();

	state.add_ope(token->ident, Operator(func, arg_num->int_value, level->int_value, left_assoc));

	return true;
}

Iteratable<LabeledBlock> Parser::parse_option_blocks()
{
	Token* keyword = nullptr;
	if (state.maybe_token(CTS("("))) {
		keyword = state.read_ident();
		state.expect_token(CTS(")"), CTS("}"));
	}
	auto stms = parse_statements();
	return Iteratable(LabeledBlock(keyword, stms));
}

VariableDeclaration* parse_bracket_var_dec()
{
	if (state.maybe_token(CTS("("))) {
		auto var_dec = parse_var_dec(Quantifier::ForAll);
		state.expect_token(CTS(")"), CTS("}"));
		return var_dec;
	}
	return nullptr;
}

Iteratable<Declaration> Parser::parse_declaration_axiom()
{

}

Iteratable<Declaration> Parser::parse_declaration_theorim()
{
	auto var_dec = parse_bracket_var_dec();
	auto body = parse_statements();
	auto options = parse_option_blocks();
	return Iteratable(DeclarationTheorim(var_dec, body, options)).cast<Declaration>();;
}

Iteratable<Declaration> Parser::parse_declaration_define()
{
	bool has_error = false;

	auto ident = state.maybe_ident();
	if (!ident) {
		state.make_error_curr("識別子がありません");
		has_error = true;
	}

	auto var_dec = parse_bracket_var_dec();

	ExpressionRef eval_type = nullptr;
	if (state.maybe_token(CTS(":"))) {
		eval_type = parse_expression();
	} else {
		state.make_error_curr("型指定が必要です");
		has_error = true;
	}

	auto body = parse_statements();
	auto options = parse_option_blocks();

	if (has_error)
		return nullptr;
	return Iteratable(DeclarationDefine(var_dec, eval_type, body, options)).cast<Declaration>();;
}

Iteratable<Declaration> Parser::parse_declaration_undef()
{

}

Iteratable<Declaration> Parser::parse_declaration_type()
{
	bool has_error = false;

	auto ident = state.maybe_ident();
	if (!ident) {
		state.make_error_curr("識別子がありません");
		has_error = true;
	}

	auto var_dec = parse_bracket_var_dec();

	ExpressionRef base = nullptr;
	if (state.maybe_token(CTS("extends")))
		base = parse_expression();

	auto blocks = parse_block_set();
	auto cond = blocks.body->get_stms_expr(state.get_msg());

	if (has_error)
		return nullptr;
	return Iteratable(DeclarationType(ident, var_dec, base, cond)).cast<Declaration>();
}

Iteratable<Declaration> Parser::parse_declaration()
{
	switch (state.token()->ident) {
	case CTS("undef"):
		return parse_declaration_undef();
	case CTS("axiom"):
		return parse_declaration_axiom();
	case CTS("def"):
		return parse_declaration_define();
	case CTS("theorim"):
		return parse_declaration_theorim();
	case CTS("type"):
		return parse_declaration_type();
	case CTS("operator"):
		parse_declaration_operator();
		return nullptr;
	default:
		state.make_error_curr();
	}
}

ProgramEntity parse_program(Iteratable<Token> token)
{
	::data = ParseSection(token);

	while (!is_ended()) {
		if (maybe_token(Annotator::EndLine))
			continue;
		parse_declaration();
	}

	ProgramEntity ent;
	ent.cfg = std::move(::cfg);
	ent.decs = std::move(::decs);
	ent.msg = std::move(::msg);
	return ent;
}

TypedExpression parse_typed_expression(Iteratable<Token> token, const OperatorConfig& cfg)
{
	::cfg = cfg;
	::msg = MessageManager();
	msg = MessageManager();

	::data = ParseSection(token);
	expect_token(CTS("("), 0);
	auto var_dec = parse_var_dec(Quantifier::ForAll);
	expect_token(CTS(")"), 0);
	auto expr = parse_expression();
	return TypedExpression(std::move(var_dec), std::move(expr), std::move(msg));
}