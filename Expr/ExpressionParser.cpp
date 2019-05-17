#include "ExpressionParser.h"

class EmbeddedStrParseState
{
private:
	Location loc;
	const char* str;
	const char* curr_begin;
	FixedArray<ExpressionRef> exprs;
	static const Token str_add;

public:
	EmbeddedStrParseState(const char* str, Location loc) : str(str), loc(loc) {}

	void make_str(int i)
	{
		int size = str + i - curr_begin;
		if (size) {
			auto token = new Token(StringView(curr_begin, size), Annotator::String, loc + i);
			exprs.push_back(Expression::create(token));
		}
	}

	void make_expr(int i)
	{
		auto token = new Token(StringView(str + i, 1), Annotator::Identifer, loc + i);
		exprs.push_back(Expression::create(token));
	}

	void update_curr(int i)
	{
		curr_begin = str + i;
	}

	ExpressionRef get_expr()
	{
		//constexpr const char* func_name = "ss";
		return Expression::create(nullptr, exprs);
	}
};

class EmbeddedStrParser : protected EmbeddedStrParseState
{
private:
	EmbeddedStrParser(const char* str, Location loc) : EmbeddedStrParseState(str, loc) {}

	ExpressionRef parse(Token* token)
	{
		bool has_var = false;
		auto str = token->str;
		for (int i = 0; i < str.size(); i++) {
			if (str[i] == '$' && i < str.size() - 1) {
				has_var = true;
				make_str(i - 1);
				make_expr(i++);
				update_curr(i++);
			}
		}
		if (has_var) {
			make_str(str.size() - 1);
			return get_expr();
		} else {
			return Expression::create(token);
		}
	}

public:
	static ExpressionRef parse(Token* token)
	{
		EmbeddedStrParser parser(token->str.data(), token->loc);
		return parser.parse(token);
	}
};

void ExpressionParseState::add_term(Token* token)
{
	if (token->ann == Annotator::String)
		push_expr(EmbeddedStrParser::parse(token));
	else
		push_expr(Expression::create(token));
}

/*
‘€ŽÔêƒAƒ‹ƒSƒŠƒYƒ€

input: -((1+2)*3)+5/4
-: | -
(: | (-
(: | ((-
1: 1 | ((-
+: 1 | +((-
2: 12 | +((-
): (12+) | (-		 // ‰‰ŽZstack‚Ìæ“ª‚ª'('‚É‚È‚é‚Ü‚Åpop&o—Ístack‚Épush‚µA'('‚Ípop(íœ)
*: (12+) | *(-
3: (12+)3 | *(-
): ((12+)3*) | -
+: (((12+)3*)-) | +	 // Ž©•ª‚æ‚è—Dæ‡ˆÊ‚Ì‚‚¢‰‰ŽZ‚ð‘S‚Äpop‚µ‚½‚ ‚Æ‚É‰‰ŽZstack‚Épush
5: (((12+)3*)-)5 | +
/: (((12+)3*)-)5 | /+
4: (((12+)3*)-)54 | /+
 : ((((12+)3*)-)(54/)+)

input: f(1+2, 3/g(4, 5), 6)
f: | f
(: | (f
1: 1 | (f
+: 1 | +(f
2: 12 | +(f
,: (12+) | (f		// ‰‰ŽZƒXƒ^ƒbƒN‚Ìæ“ª‚ª'('‚É‚È‚é‚Ü‚ÅpopA'('‚Ípop‚µ‚È‚¢
3: (12+)3 | (f          // ¨ŠÖ”î•ñ‚Ìˆø”‚Ì”‚ðƒCƒ“ƒNƒŠƒƒ“ƒg‚·‚é
/: (12+)3 | /(f
g: (12+)3 | g/(f	// ŠÖ”‚æ‚è—Dæ‡ˆÊ‚Ì‚‚¢‰‰ŽZ‚Í‚È‚¢
(: (12+)3 | (g/(f
4: (12+)34 | (g/(f
,: (12+)34 | (g/(f
5: (12+)345 | (g/(f
): (12+)345 | g/(f
,: (12+)3(45g) | /(f
 : (12+)(3(45g)/) | (f
6: (12+)(3(45g)/)6 | (f
): (12+)(3(45g)/)6 | f
 : ((12+)(3(45g)/)6)f)

 */

ExpressionRef ExpressionParser::parse(ParsingState* state)
{
	while (!is_ended() && (!state->is_ended() || state->token()->ann == Annotator::Operator)) {
		switch (state->token()->ann) {
		case Annotator::Integer:
			add_term(state->token());
			break;
		case Annotator::Identifer:
			// ‰‰ŽZŽq
			if (auto ope = state->get_ope(state->token())) {
				make_expr_to_preceder(*ope);
				state->token()->ope = ope;
				add_ope(state->token());
				// ŠÖ”
			} else if (auto next = state->next_token(); next && next->ident == CTS("(")) {
				add_func(state->token());
				state->next_token();
				add_bracket(state->token());
				// Ž¯•ÊŽq
			} else {
				add_term(state->token());
			}
			break;
		case Annotator::Operator:
			if (auto ope = state->get_ope(state->token())) {
				make_expr_to_preceder(*ope);
				state->token()->ope = ope;
				add_ope(state->token());
			} else {
				state->add_error("•s–¾‚È‰‰ŽZŽq‚Å‚·");
				goto END_PARSE;
			}
			break;
		case Annotator::Symbol:
			switch (state->token()->ident) {
			case CTS(","):
				increment_func_arg();
				make_expr_to_bracket();
				break;

			case CTS(")"):
				make_expr_to_bracket();
				pop_bracket();
				break;

			case CTS("("):
				add_tuple_bracket(state->token());
				break;

			default:
				goto END_PARSE;
			}
			break;

		default:
			goto END_PARSE;
		}

		state->next_token();
	}

END_PARSE:
	make_expr_all();
	return get_expr();
}