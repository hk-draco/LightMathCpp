#pragma once
#include "Token.h"
#include "RewritingInfo.h"


struct FunctionProperty
{
	bool is_commutative;
	bool is_associative;

	FunctionProperty() : is_commutative(false), is_associative(false) {}
};

enum class ExpressionType
{
	None,
	Step,   // [�ŏ��̎�, ..., �ŐV�̎�]
	Lambda, // [�����ϐ�, �{��]
	Constant, Variable,		 // [(��)]
	Function, CFunction, ACFunction, // [����1, ..., ����n]
	Type,         // [����1, ..., ����n]
	FunctionType, // [[����1�̌^, ..., ����n�̌^], [�߂�l�̌^]]
	TypeArgs
};

struct ACRuleMaking
{
	ExpressionRef before_bind_var;
	ExpressionRef after_bind_var;
};

enum class ExpressionShowMode
{
	Latest, Step
};

class Expression
{
private:
	ExpressionType type;
	RewritingInfo* rewriting;
	const ExpressionRef eval_type;

	union
	{
		std::intptr_t ident;
		Token* head;
		Object* obj;
		Variable* var;
		ExpressionRef parent_step;
		int bind_var;
	};


	static void get_exprs(StringView func_name, RefArray<Expression> args, FixedArray<Expression>* buf);

	void make_func(const ExpressionRef eval_type, std::intptr_t ident, ExpressionType type)
	{
		this->type = type;
		this->eval_type = eval_type;
		this->ident = ident;
	}

	template<typename TFunc>
	ExpressionRef find_unique_arg(TFunc cond)
	{
		ExpressionRef res = nullptr;
		for (auto arg : Range(args)) {
			if (cond(arg)) {
				if (res)
					return nullptr;
				res = arg;
			}
		}
		return res;
	}

	template<typename TFunc>
	ExpressionRef find_arg(TFunc cond)
	{
		for (auto arg : Range(args)) {
			if (cond(arg))
				return arg;
		}
		return nullptr;
	}

	Expression() {}
	Expression(Token* head, int arg_size) 
		: head(head), args(arg_size), type(ExpressionType::None) {}
	Expression(ExpressionType type, Token* head, std::intptr_t ident, RefArray<Expression> args)
		: type(type), head(head), ident(ident), args(args) {}

	bool unify_lambda_args(AssignTable* asg, const ExpressionRef trg) const;

	void make_step(ExpressionRef new_expr);

public:
	void marge_steps(ExpressionRef trg);
	static ExpressionRef* get_latest_step(ExpressionRef* trg);

	static bool commute_ac(ExpressionRef* trg, ExpressionRef rewrote_arg);
	static void rewrite(ExpressionRef* trg, ExpressionRef new_expr, RewritingInfo* info = nullptr);
	static void marge_steps(ExpressionRef* trg2, ExpressionRef steps);

	static ExpressionRef create(Token* head, int arg_size = 0)
	{
		return new(arg_size) Expression(head, arg_size);
	}

	void set_arg(int idx, ExpressionRef arg)
	{
		args[idx] = arg;
	}

	void complete(const GlobalScope& glb, const Scope& scope);


	//Expression(List<Expression>&& args) : args(std::move(args)), parent_step(nullptr) {}

	// bind_var���g�p�����Ƃ�true��Ԃ�
	static bool make_ac_rule(ExpressionRef before, ExpressionRef* after_ent, int bind_var);


	// �N���[�����쐬����B
	ExpressionRef clone_latest() const;


	// �����ϐ�����������������Ԃ��B
	// �����_���̏ꍇ�̂ݎg�p�\�B
	ExpressionRef rename_bind_var(ExpressionRef new_var);


	//======================================================================
	// �u������
	//======================================================================

	// �ϐ�[ident]����[expr]�̃N���[���Ɉ�x�����u��������B
	// �u��������Ώۂ��Ȃ��Ƃ���u��������Ώۂ�2�ȏ゠��Ƃ��͒u����������nullptr��Ԃ��B�u�������ɐ��������Ƃ��́A�u����������̎���Ԃ��B
	// ��) this=a*(x+y)^2, ident=x, expr=f(x) => this=a*(f(x)+y)^2, return=this��f(x)�̃|�C���^�[
	ExpressionRef replace_once(std::intptr_t ident, const ExpressionRef expr);

	void replace(std::intptr_t ident, ExpressionRef expr);

	//======================================================================
	// ����
	//======================================================================

	// ��[trg]�̓����̎�[trg_child]�Ɠ����ʒu�̂��̎��̓����̎���Ԃ��B
	// �����ʒu�̎���step�ł���Ƃ��́Astep���̍ł��V�������ł͂Ȃ��Astep���̂�Ԃ��B
	// ��[trg_child]��step�ł���Ƃ��́A����step�̒��̍ł��V���������w�肵���̂Ɠ������ʂ�Ԃ��B
	// ���̎��Ǝ�[trg]�͒P�ꉻ�\�ł���K�v������A��[trg]�͎�[trg_child]���܂�ł���K�v������B
	// ���̑O���������������Ă��Ȃ��Ƃ��́Anullptr��Ԃ��B
	// ��) this=x+y>f(z, w), trg=a+b>f(c, d), trg_child=c => return=this��z�̃|�C���^�[
	ExpressionRef find(const ExpressionRef trg, const ExpressionRef trg_child);

	struct ExpressionDifference find_diff(const ExpressionRef trg) const;

	//======================================================================
	// Step�֌W
	//======================================================================

	// �ł��V����Step��Ԃ��B
	ExpressionRef get_latest_step();

	// ����������B
	void rewrite(ExpressionRef new_expr, RewritingInfo rewriting);

	// �X�e�b�v���t�ɂ���B
	void reverse_steps();

	// �X�e�b�v�𓝍�����B
	// [A, B, C, D] + [D, P, Q] => [A, B, P, Q]
	void marge_steps(ExpressionRef trg);

	std::tuple<bool, std::vector<ExpressionRef>> has_step(ExpressionRef trg);

	void cut_next()
	{
		ZeroMemory(this, sizeof(std::intptr_t));
	}

	//======================================================================
	// Getter
	//======================================================================

	const ExpressionRef eval() const
	{
		return eval_type;
	}

	class Object* get_obj() const
	{
		assert(type == ExpressionType::Function || type == ExpressionType::CFunction || type == ExpressionType::ACFunction);
		return (class Object*)ident;
	}

	class Type* get_type() const
	{
		assert(type == ExpressionType::Type);
		return (class Type*)ident;
	}

	Token* get_head() const
	{
		return head;
	}

	Token* get_func_name() const
	{
		return head->ope.get_func_name();
	}

	Area get_area() const
	{
		if (!args)
			return head->area();
		return Area(head->begin(), args.back()->get_area().end);
	}

	RefArray<ExpressionRef> get_args() const
	{
		return args;
	}

	std::intptr_t get_ident() const
	{
		return ident;
	}

	bool is_var() const
	{
		return type == ExpressionType::Variable;
	}

	bool is_func() const
	{
		return type == ExpressionType::Function || type == ExpressionType::CFunction || type == ExpressionType::ACFunction;
	}

	bool is_ac_func() const
	{
		return type == ExpressionType::ACFunction;
	}

	void write(std::string* buf, ExpressionShowMode mode) const;

	std::string to_str() const
	{
		std::string str;
		write(&str, ExpressionShowMode::Latest);
		return str;
	}
};
