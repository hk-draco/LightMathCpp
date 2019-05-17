#pragma once
#include "Expression.h"
#include "Scope.h"
#include "Message.h"
#include "RewritingInfo.h"

bool simplify(ExpressionRef trg, ExpressionRef goal_expr = nullptr);

struct TypedExpression
{
	VariableDeclaration* var_dec;
	ExpressionRef expr;
	MessageManager msg;
	bool has_error;

	TypedExpression(VariableDeclaration* var_dec, ExpressionRef expr, MessageManager&& msg)
		: var_dec(var_dec), expr(expr), msg(std::move(msg)), has_error(false)
	{}
};

struct RunningState
{
	Scope* scope;
	MessageManager* msg;
	ExpressionRef* trg;
};

class Statement
{
public:
	virtual void run(RunningState* state) = 0;
};

class ExprStatement : public Statement
{
protected:
	ExpressionRef expr;

public:
	ExprStatement(ExpressionRef expr) : expr(expr) {}
};

class StatementStep : public ExprStatement
{
private:
	

public:
	StatementStep(ExpressionRef expr) : ExprStatement(expr) {}

	void run(RunningState* state) override;
};

class StatementImpl : public ExprStatement
{
private:

public:
	StatementImpl(ExpressionRef expr) : ExprStatement(expr) {}

	void run(RunningState* state) override;
};

class StatementUnfold : public ExprStatement
{
private:

public:
	StatementUnfold(ExpressionRef expr) : ExprStatement(expr) {}

	void run(RunningState* state) override;
};

enum class TargetType
{
	Left, Right, All, Context
};

class StatementTarget : public Statement
{
private:
	TargetType type;

public:
	StatementTarget(TargetType type) : type(type) {}

	void run(RunningState* state) override;
};

class BlockStatement : public Statement
{
protected:
	Iteratable<Statement> stms;

public:
	BlockStatement(Iteratable<Statement> stms) : stms(stms) {}
};

class StatementFork : public BlockStatement
{
private:

public:
	StatementFork(Iteratable<Statement> stms) : BlockStatement(stms) {}
	void run(RunningState* state) override;
};

class StatementWhere : public BlockStatement
{
private:	
	Area area;
	ExpressionRef arg;
	
public:
	StatementWhere(Iteratable<Statement> stms) : BlockStatement(stms) {}
	void run(RunningState* state) override;
};

class StatementWhen : public BlockStatement
{
	
};

class StatementImplies : public BlockStatement
{

};

class StatementVariable : public Statement
{
private:
	VariableDeclaration* var_dec;

public:
	StatementVariable(VariableDeclaration* var_dec) : var_dec(var_dec) {}

	void run(RunningState* state) override;
};

class StatementExpression : public Statement
{
private:
	ExpressionRef expr;

public:
	StatementExpression(ExpressionRef expr) : expr(expr) {}

	void run(RunningState* state) override;
};

enum class LabelType
{
	Body, Name, Latex, Proof, Desc
};

class StatementLabel : public Statement
{
private:

public:
	StatementLabel()
};