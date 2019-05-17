#pragma once
#include "Token.h"
#include "Statement.h"
#include "Declaration.h"
#include "Message.h"
#include "ParsingState.h"

class Parser
{
private:
	ListBuilder<Declaration> decs;
	ParsingState state;

	template<class TStatement>
	Iteratable<Statement> parse_expr_statement()
	{
		if (auto expr = parse_expression())
			return Iteratable(TStatement(expr)).cast<Statement>();
		return nullptr;
	}

	template<class TStatement>
	Iteratable<Statement> parse_keyword_statement()
	{
		if (auto keyword = data.read_ident())
			return Iteratable(TStatement(keyword)).cast<Statement>();
		return nullptr;
	}

	template<class TStatement>
	Iteratable<Statement> parse_block_statement()
	{
		if (auto stms = parse_statements())
			return Iteratable(TStatement(stms)).cast<Statement>();
		return nullptr;
	}

	Iteratable<Statement> parse_var_dec_statement(Quantifier qtf)
	{
		if (auto var_dec = parse_variable_declaration(qtf))
			return Iteratable(StatementVariable(var_dec)).cast<Statement>();
		return nullptr;
	}

	template<class TStatement>
	Iteratable<Statement> parse_expr_block_statement()
	{
		if (auto arg = state.parse_expression()) {
			if (auto stms = parse_statement_block())
				return Iteratable(TStatement(arg, stms)).cast<Statement>();
		}
		return nullptr;
	}

	template<class TStatement>
	Iteratable<Statement> parse_block_statement()
	{
		if (auto arg = state.parse_expression()) 
			return Iteratable(TStatement(stms)).cast<Statement>();
		return nullptr;
	}

public:
	void parse_idents(FixedArray<Token*>* out);
	ListBuilder<VariableTyping> parse_variable_typings_per_type(bool has_ref_vars);
	ListBuilder<VariableTyping> parse_variable_typings(Quantifier qtf);
	VariableDeclaration* parse_variable_declaration(Quantifier qtf);

	Iteratable<Statement> parse_statements();
	Iteratable<Statement> parse_statement();

	Iteratable<Statement> parse_statement_block();
	
	VariableDeclaration* parse_variable_declaration(Quantifier qtf);

	Iteratable<LabeledBlock> parse_option_blocks();

	Iteratable<Declaration> parse_declaration();
	Iteratable<Declaration> parse_declaration_type();
	Iteratable<Declaration> parse_declaration_undef();
	Iteratable<Declaration> parse_declaration_define();
	Iteratable<Declaration> parse_declaration_axiom();
	Iteratable<Declaration> parse_declaration_theorim();
	bool parse_declaration_operator();
};