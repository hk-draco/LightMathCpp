#pragma once

class ParsingState
{
private:
	Iteratable<Token> curr;
	MessageManager* msg;
	std::map<Ident, Operator> opes;

public:
	ParsingState(Iteratable<Token> curr, MessageManager* msg) : curr(curr), msg(msg) {}	
	
	Operator* get_ope(Token* token)
	{
		auto itr = opes.find(token->ident);
		if (itr == opes.end())
			return nullptr;
		return &itr->second;
	}

	void add_ope(Ident ident, Operator ope)
	{
		opes[ident] = ope;
	}

	MessageManager* get_msg()
	{
		return msg;
	}
	
	bool is_ended()
	{
		return !curr;
	}

	void step_token()
	{
		curr = curr.next();
	}

	void skip_end_line()
	{
		while (curr->ann == Annotator::EndLine)
			step_token();
	}

	// 任意のトークン[ident]が来た場合は次のトークンに進みtrueを返し、来なかった場合は次のトークンには進まずfalseを返す
	bool maybe_token(Ident ident)
	{
		if (curr->ident != ident)
			return false;
		step_token();
		return true;
	}

	bool maybe_token(Annotator ann)
	{
		if (curr->ann != ann)
			return false;
		step_token();
		return true;
	}

	Token* maybe_ident()
	{
		if (curr->ann != Annotator::Identifer && curr->ann != Annotator::Integer)
			return nullptr;
		auto res = token();
		step_token();
		return res;
	}

	template<typename TFunc1, typename TFunc2>
	bool expect_token(Ident end, TFunc1 msg_gen, TFunc2 expect_cond)
	{
		auto start = curr, prev = curr;
		while (!is_ended() && !expect_cond(curr.front()) && curr->ident != end) {
			prev = curr;
			step_token();
		}
		if (start != prev) {
			msg.add_err(msg_gen(), Area(start->begin(), prev->end()));
			return false;
		}
		if (is_ended())
			return false;
		step_token();
		return true;
	}

	// 任意のトークン[ident]が来た場合は次のトークンに進みtrueを返し、来なかった場合はトークンが終了するか任意のトークン[end]が来るまで進み、
	// その範囲の領域でエラー[msg]を記録した後、falseを返す。
	bool expect_token(Ident ident, Ident end, const char* msg)
	{
		return expect_token(end, [=]() {return msg; }, [=](Token* trg) {return trg->ident == ident; });
	}

	bool expect_token(Ident ident, Ident end)
	{
		auto msg_gen = [=]() {
			std::string msg = "'";
			msg += std::string((char*)&ident);
			msg += "'がありません";
			return msg;
		};
		return expect_token(end, msg_gen, [=](Token* trg) {return trg->ident == ident; });
	}

	bool expect_end_line()
	{
		return expect_token(0, [&]() {return "改行がありません"; }, [](Token* trg) {return trg->ann == Annotator::EndLine; });
	}

	Token* token()
	{
		return curr.front();
	}

	Token* next_token()
	{
		return curr.next().front();
	}

	// 今のトークンを返し、次に進む
	Token* read_token()
	{
		auto res = curr.front();
		step_token();
		return res;
	}

	Token* read_token(Ident ident)
	{
		if (curr->ident != ident)
			return nullptr;
		return read_token();
	}

	Token* read_token(Annotator ann)
	{
		if (curr->ann != ann)
			return nullptr;
		return read_token();
	}

	Token* read_ident()
	{
		return read_token(Annotator::Identifer);
	}

	void add_error(const char* str)
	{
		msg->add_err(str, curr.front()->area());
	}

	void make_error_curr(const char* str = "不明なトークンです")
	{
		msg->add_err(str, curr.front()->area());
		step_token();
	}
};