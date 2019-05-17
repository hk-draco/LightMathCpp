#pragma once
#include "Token.h"

enum class ResultType
{
	ProofTrace, RewritingRule, Message, Error
};

class Message
{
private:
	Area area;
	ResultType type;
	const char* msg;
	std::array<StringView, 3> ent_ptrs;
	std::array<String, 3> ents;
	int ent_num;

	void write_type(String* buf) const
	{
		switch (type) {
		case ResultType::ProofTrace:
			buf->append("proof");
			break;
		case ResultType::RewritingRule:
			buf->append("rule");
			break;
		case ResultType::Message:
			buf->append("msg");
			break;
		case ResultType::Error:
			buf->append("error");
			break;
		default:
			assert(false);
		}
	}

	void write_ent_type(String* buf, char type) const
	{
		switch (type) {
		case 'T':
			buf->append("型'");
			break;
		case 'V':
			buf->append("変数'");
			break;
		case 'E':
			buf->append("式'");
			break;
		case 'F':
			buf->append("関数'");
			break;
		case 'A':
			buf->append("引数'");
			break;
		case 'M':
			buf->append("トークン'");
			break;
		case 'I':
			buf->append("識別子'");
			break;
		case 'O':
			buf->append("演算子'");
			break;
		default:
			assert(false);
		}
	}

	void write_ent(String* buf, int idx) const
	{
		if (ents[idx])
			buf->append(ents[msg[idx]]);
		else
			buf->append(ent_ptrs[idx]);
	}

	void write_msg(String* buf) const
	{
		int ent_idx = 0;
		for (int i = 0; msg[i] != '\0'; i++) {
			if (msg[i] == '$') {
				i++;
				write_ent_type(buf, msg[i]);
				write_ent(buf, ent_idx);
				ent_idx++;
				buf->push_back('\'');
			} else {
				buf->push_back(msg[i]);
			}
		}
	}

public:
	Message() {}
	Message(ResultType type, Area area, const char* str)
		: type(type), ent_num(0), area(area), msg(str)
	{}

	void add(StringView str)
	{
		ent_ptrs[ent_num] = str;
		ent_num++;
	}

	void add(String str)
	{
		ents[ent_num] = str;
		ent_num++;
	}

	void write(String* buf) const
	{
		buf->append("{");
		buf->append("\"area\":");
		area.write(buf);
		buf->append(",\"type\":\"");
		write_type(buf);
		buf->append("\",\"msg\":\"");
		write_msg(buf);
		buf->append("\"}");
	}
};

class MessageManager
{
private:
	ListBuilder<Message> msgs;

public:
	Message* add_err(const char* msg, Area area)
	{
		msgs.push_back(Message(ResultType::Error, area, msg));
		return msgs.back();
	}

	Message* add_msg(const char* msg, Area area)
	{
		msgs.push_back(Message(ResultType::Message, area, msg));
		return msgs.back();
	}

	void write(String* buf) const
	{
		buf->push_back('[');
		for (auto[msg, idx] : msgs.range().indexed()) {
			if(idx)
				buf->push_back(',');
			msg->write(buf);
		}
		buf->push_back(']');
	}
};