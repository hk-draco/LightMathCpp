#include "funcs.h"
#include "rewrite.h"
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

std::string run(const char* code, Scope* scope, OperatorConfig* cfg)
{
	auto tokens = lexical_analyze(code);
	auto prg = parse_program(tokens);
	*glb = analyze_program(&prg.msg, prg);

	std::string msg;
	prg.msg.write(&msg);

	*cfg = std::move(prg.cfg);
	return msg;
}

Expression* create_expr(const char* code, Scope* scope, const OperatorConfig& cfg)
{
	MessageManager msg;
	auto tokens = lexical_analyze(code);
	auto expr = parse_typed_expression(tokens, cfg);
	make_expression(&expr, glb, &msg);
	if (expr.has_error)
		return nullptr;
	return expr.expr.get();
}

struct Test
{
	std::string code;
	GlobalScope glb;
	OperatorConfig cfg;

	Test() : glb(nullptr) {}

	void run()
	{
		::run(code.data(), &glb, &cfg);
	}

	void run(std::string str, int count=1)
	{
		clock_t start_time = clock();

		std::string log, msg, start, goal;
		for (int i = 0; i < count; i++) {
			log = ::run(code.data(), &glb, &cfg);
			if (auto expr = create_expr(str.data(), &glb, cfg)) {
				expr->write(&start, ExpressionShowMode::Latest);
				simplify(expr);
				msg.clear();
				expr->write(&msg, ExpressionShowMode::Step);
				expr->write(&goal, ExpressionShowMode::Latest);
			}
		}

		auto time = ((double)(clock() - start_time)) / (double)count;

		printf(start.data());
		printf(" => ");
		printf(goal.data());
		printf("\n\n");
		printf(log.data());
		printf("\n\n");
		printf(msg.data());
		printf("\n\n%fms\n", time);
		printf("====================================================\n");
	}

	void push_code1()
	{
		code += "\
		type R				\n\
		operator +(2, 2) => add		\n\
		operator =(2, 1) => equals	\n\
		operator =>(2, 1) => simpl	\n\
		undef add(a, b: R): R		\n\
		axiom (a, b: R) {		\n\
			a + b = b + a		\n\
		}				\n\
		axiom (a, b, c: R) {		\n\
			a + (b + c) = (a + b) + c \n\
		}				\n\
		undef 0: R			\n\
		axiom (a: R) {			\n\
			a + 0 => a		\n\
		}				\n\n";
	}

	void run_test1()
	{
		run("(x: R) 0+0+x+0+0+0");
		auto obj = glb.get_obj(StringView("add", 3));
		assert(obj);
		auto prop = obj->get_prop();
		assert(prop.is_commutative);
		assert(prop.is_associative);
	}

	void push_code2()
	{
		code += "\
		undef sin(x: R): R	\n\
		undef cos(x: R): R	\n\
		undef 1: R		\n\
		axiom (x: R) {		\n\
			sin(x) + cos(x) => 1 \n\
		}			\n\n";
	}

	void run_test2()
	{
		run("(x: R) x+x+cos(x)+cos(x)+x+sin(x)+cos(x)+x+cos(x)+x+sin(x)+sin(x)");
	}

	void load_file(const char* filename)
	{
		std::ifstream ifs(std::string("C://Dropbox//LightMath//LightMath//MathEngine//") + filename);
		auto str = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		code = str;
	}
};

int main()
{
	Test test;
	//test.push_code1();
	//test.push_code2();
	//test.run_test2();
	test.load_file("test.lm");
	test.run();
	//test.run("(x: R) (x*(1+0)+1*x+(0*1+x+0)*1)*1");
	//test.run("(A, B: Prop) not(not(A)) | B & A | B");
	//test.run("(A, B, P, Q: Prop) ((A -> P) & (B -> Q)) -> ((A & B) -> (P & Q))");

	return 0;
}