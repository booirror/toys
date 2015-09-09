//
//  jsonar.cpp
//  testcpp11
//
//  Created by booirror on 15-3-24.
//  Copyright (c) 2015 booirror. All rights reserved.
//

#include "jsonar.h"
#include <iostream>

namespace jsonar {
	Json::~Json()
	{
		delValue(value);
		delete value;
	}

	void Json::delValue(jsonar::Value *v)
	{
		if (v == nullptr) return;
		if (v->getType() == ValueType::VT_ARR) {
			for (auto x : v->asArray()->getVal())
				delValue(x);
		}
		else if (v->getType() == ValueType::VT_OBJ){
			for (auto x : v->asObject()->getVal())
				delValue(x.second);
		}
		else {
			delete v;
		}
	}

	Value* Json::parse() {
		if (lookhead('{')) {
			Object* obj = new Object();
			try {
				matchObject(obj);
			}
			catch (InvalidJson& e) {
				delValue(obj);
				throw e;
			}
			if (!match('\0')) {
				delValue(obj);
				throw InvalidJson("expecting EOF", getInfo());
			}
			value = obj;
			return obj;
		}
		else if (lookhead('[')) {
			Array *arr = new Array();
			try {
				matchArray(arr);
			}
			catch (InvalidJson & e) {
				delValue(arr);
				throw e;
			}
			if (!match('\0')) {
				delValue(arr);
				throw InvalidJson("expecting EOF", getInfo());
			}
			value = arr;
			return arr;
		}
		else {
			throw InvalidJson("expecting {,[", getInfo());
		}
		return NULL;
	}

	bool Json::matchObject(Object* obj) {
		bool canEmpty = true;
		if (!match('{')) {
			throw InvalidJson("expecting {");
		}
		do {
			try {
				if (!matchPair(obj, canEmpty)) {
					throw InvalidJson("expecting obj", getInfo());
				}
			}
			catch (InvalidJson& e) {
				throw e;
			}
			canEmpty = false;
		} while (match(','));
		if (!match('}')) {
			throw InvalidJson("expecting }");
		}
		return true;
	}

	bool Json::matchPair(jsonar::Object *obj, bool canEmpty) {
		if (lookhead('\"')) {
			std::string s;
			if (!matchString(s)) {
				throw InvalidJson("expecting string, }");
			}
			if (!match(':')) {
				throw InvalidJson("expecting :");
			}
			Value *v = NULL;
			if (!matchValue(&v, false)) {
				throw InvalidJson("expecting value");
			}
			String k(s.c_str());
			obj->add(k, v);
			return true;
		}
		else if (lookhead('}') && canEmpty) {
			return true;
		}
		return false;
	}

	bool Json::matchArray(jsonar::Array *arr) {
		bool canEmpty = true;
		if (!match('[')) {
			throw InvalidJson("expecting [");
		}
		do {
			try {
				Value *v = NULL;
				if (!matchValue(&v, canEmpty)) {
					throw InvalidJson("expecting array,]");
				}
				if (v) {
					arr->push_back(v);
				}
			}
			catch (InvalidJson& e) {
				throw e;
			}
			canEmpty = false;
		} while (match(','));
		if (!match(']')) {
			throw InvalidJson("expecting ]");
		}
		return true;
	}

	bool Json::matchString(std::string &s) {
		pos++;
		while (data[pos] != '\0') {
			if (data[pos] == '\"') {
				pos++;
				return true;
			}
			else if (data[pos] == '\\'){
				s.append(data.substr(pos++, 1));
			}
			s.append(data.substr(pos++, 1));
		}
		return false;
	}

	bool Json::matchValue(jsonar::Value **value, bool canEmpty)
	{
		if (lookhead('\"')) {
			std::string str;
			if (!matchString(str)) {
				throw InvalidJson("expecting string");
			}
			String *s = new String(str.c_str());
			*value = s;
		}
		else if (isInSeqence("-0123456789")) {
			double d;
			try {
				if (!matchNum(d)) {
					throw InvalidJson("expecting num.");
				}
			}
			catch (InvalidJson& e) {
				throw e;
			}
			Number * num = new Number(d);
			*value = num;
		}
		else if (lookhead('{')) {
			Object * obj = new Object();
			try {
				if (!matchObject(obj)) {
					delete obj;
					throw InvalidJson("expecting obj,matchValue");
				}
			}
			catch (InvalidJson& e) {
				delete obj;
				throw e;
			}
			*value = obj;
		}
		else if (lookhead('[')) {
			Array *arr = new Array();
			try {
				if (!matchArray(arr)) {
					delete arr;
					throw InvalidJson("expecting array");
				}
			}
			catch (InvalidJson& e) {
				delete arr;
				throw e;
			}
			*value = arr;
		}
		else if (lookhead('t')) {
			if (!matchWord("true")) {
				throw InvalidJson("expecting value/true");
			}
			Bool * b = new Bool(true);
			*value = b;
		}
		else if (lookhead('f')) {
			if (!matchWord("false")) {
				throw InvalidJson("expecting value/false");
			}
			Bool * b = new Bool(false);
			*value = b;
		}
		else if (lookhead('n')) {
			if (!matchWord("null")) {
				throw InvalidJson("expecting value/null");
			}
			Null *null = new Null();
			*value = null;
		}
		else if (!isInSeqence(",}]") || !canEmpty) {
			throw InvalidJson("expecting ,}]");
		}
		else {
			*value = nullptr;
		}
		return true;
	}

	bool Json::matchNum(double &d)
	{
		int begin = pos;
		int len = 0;
		if (match('-')) {
			len++;
		}
		if (match('0')) {
			len++;
			if (!isInSeqence(".}],Ee")) {
				throw InvalidJson("expecting .}],Ee");
			}
		}
		else if (isInSeqence("123456789")) {
			while (isInSeqence("0123456789")) {
				pos++;
				len++;
			}
			if (!isInSeqence(".Ee]},")) {
				throw InvalidJson("expecting .Ee]},");
			}
		}
		else {
			return false;
		}
		if (match('.')) {
			len++;
			if (!isInSeqence("0123456789")) {
				throw InvalidJson("expecting 0123456789");
			}
			while (isInSeqence("0123456789")) {
				pos++;
				len++;
			}
		}
		if (isInSeqence("Ee")) {
			pos++; len++;
			if (isInSeqence("+-")) {
				pos++; len++;
			}
			if (!isInSeqence("0123456789")) {
				throw InvalidJson("expecting 0123456789");
			}
			while (isInSeqence("0123456789")) {
				pos++;
				len++;
			}
		}
		if (!isInSeqence("}],")) {
			throw InvalidJson("expecting }],");
		}
		std::string subs = data.substr(begin, len);
		d = std::stod(subs.c_str());
		return true;
	}

	bool Json::match(char c){
		if (lookhead(c)) {
			pos++;
			return true;
		}
		return false;
	}

	bool Json::matchWord(std::string w)
	{
		int step = 0;
		for (int i = 0; i < w.size(); ++i, step++)
		{
			if (data[pos + step] != w[i])
				return false;
		}
		pos += step;
		return true;
	}

	bool Json::isInSeqence(std::string seq)
	{
		skipws();
		for (int i = 0; i < seq.size(); ++i) {
			if (seq[i] == data[pos])
				return true;
		}
		return false;
	}

	bool Json::lookhead(char c)
	{
		skipws();
		return data[pos] == c;
	}

	void Json::skipws()
	{
		while (data[pos] == '\t' || data[pos] == ' ' || data[pos] == '\n')
			pos++;
	}

	const char* Json::getInfo() {
		static char buff[20];
		int i = 0;
		buff[i++] = '<';
		std::string temp = data.substr(pos, data.size() - pos > 10 ? 10 : std::string::npos);
		for (auto c : temp) {
			buff[i++] = c;
		}
		buff[i++] = '>';
		buff[i] = 0;
		return buff;
	}
}