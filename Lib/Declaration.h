#pragma once
#include "Expression.h"
#include "Block.h"
#include "Statement.h"

struct VariableTyping
{
	ExpressionRef type;
	Token* ident;
	MemberArray<Token*> ref_idents;

	VariableTyping(Token* ident)
		: ident(ident)
	{}

	VariableTyping(Token* ident, const FixedArray<Token*>& ref_idents)
		: ident(ident), ref_idents(ref_idents)
	{}
};

struct VariableDeclaration
{
	Quantifier qtf;
	Iteratable<VariableTyping> typings;

	VariableDeclaration(Quantifier qtf, Iteratable<VariableTyping> typings)
		: qtf(qtf), typings(typings)
	{}
};

class Declaration
{
private:
	ListBuilder<LabeledBlock> options;

public:
	Declaration(ListBuilder<LabeledBlock> options) : options(options) {}

	virtual void run(Scope* scope) = 0;
};

class DeclarationProp : public Declaration
{
protected:
	ExpressionRef prop;
	VariableDeclaration* var_dec;

public:
	DeclarationProp(VariableDeclaration* var_dec, ExpressionRef prop) : var_dec(var_dec), prop(prop) {}

	void add_prop(Scope* scope);
};

class DeclarationTheorim : public DeclarationProp
{
private:
	Scope scope;
	Area area;
	Token* name;
	Iteratable<Statement> body;
	VariableDeclaration* var_dec;

public:
	DeclarationTheorim(VariableDeclaration* var_dec, Iteratable<Statement> body, ListBuilder<LabeledBlock> options)
		: var_dec(var_dec), body(body), DeclarationProp(options)
	{}

	void run(Scope* scope) override;
};

class DeclarationAxiom : public DeclarationProp
{
private:
	ExpressionRef prop;
	VariableDeclaration* var_dec;

public:
	DeclarationAxiom(VariableDeclaration* var_dec, ExpressionRef prop, ListBuilder<LabeledBlock> options) 
		: var_dec(var_dec), prop(prop), Declaration(options)
	{}

	void run(Scope* scope) override;
};

class DeclarationDefine : public Declaration
{
private:
	ExpressionRef eval_type;
	VariableDeclaration* var_dec;
	ExpressionRef body;

public:
	DeclarationDefine(VariableDeclaration* var_dec, ExpressionRef eval_type, ExpressionRef body, ListBuilder<LabeledBlock> options)
		: var_dec(var_dec), eval_type(eval_type), body(body), Declaration(options)
	{}

	void run(Scope* scope) override;
};

class DeclarationUndefinedTerm : public Declaration
{
private:
	ExpressionRef eval_type;
	VariableDeclaration* var_dec;

public:
	DeclarationUndefinedTerm(VariableDeclaration* var_dec, ExpressionRef eval_type, ListBuilder<LabeledBlock> options)
		: var_dec(var_dec), eval_type(eval_type), Declaration(options)
	{}

	void run(Scope* scope) override;
};

class DeclarationType : public Declaration
{
private:
	Token* ident;
	ExpressionRef base;
	ExpressionRef cond;
	VariableDeclaration* var_dec;

public:
	DeclarationType(Token* ident, VariableDeclaration* var_dec, ExpressionRef base, ExpressionRef cond)
		: ident(ident), var_dec(var_dec), base(base), cond(cond)
	{}

	void run(Scope* scope) override;
};