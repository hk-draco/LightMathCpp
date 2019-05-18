#pragma once
//====================================================================================================
// �R�[�h��̊T�O��\������N���X
//====================================================================================================

#include "token.h"
#include "expr.h"

//======================================================================
// �ϐ��錾
//======================================================================

//======================================================================
// �X�e�[�g�����g
//======================================================================

struct Statement
{
	Area area;
	StatementType type;
	ExpressionRef arg;
	bool is_label;

	VariableDeclaration* var_dec;
	Iteratable<Statement> stms;
	Token* keyword;

	Statement(Area area, StatementType type, ExpressionRef arg, Iteratable<Statement> stms)
		: area(area), type(type), arg(std::move(arg)), stms(std::move(stms)), is_label(false)
	{}

	Statement(Area area, StatementType type, ExpressionRef arg, bool is_label)
		: area(area), type(type), arg(std::move(arg)), is_label(is_label)
	{}

	Statement(Area area, VariableDeclaration* var_dec)
		: area(area), type(StatementType::Variable), var_dec(std::move(var_dec)), is_label(false)
	{}

	Statement(Area area, StatementType type, Token* keyword, bool is_label)
		: area(area), type(type), keyword(keyword), is_label(is_label)
	{}

	Statement(ExpressionRef arg)
		: area(arg->get_area()), arg(std::move(arg)), type(StatementType::Expression), is_label(false)
	{}
};

//======================================================================
// ���x���t���u���b�N
//======================================================================

//======================================================================
// �錾
//======================================================================
enum class DeclarationType
{
	Axiom, Theorim, Definition, Type, UndefinedTerm, Operator
};
struct Declaration
{
	DeclarationType type;
	Area area;
	Token* name;
	VariableDeclaration* var_dec;
	ExpressionRef eval_type;
	BlockSet blocks;

	Declaration(DeclarationType type, Area area) : type(type), area(area) {}
};

//======================================================================
// ���Z�q�Ǘ��N���X
//======================================================================

//======================================================================
// �v���O����
//======================================================================
struct ProgramEntity
{
	OperatorConfig cfg;
	MessageManager msg;
	Iteratable<Declaration> decs;
};