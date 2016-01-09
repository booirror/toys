//
//  jsonar.h
//  testcpp11
//
//  Created by booirror on 15-3-24.
//  Copyright (c) 2015 booirror. All rights reserved.
//

#ifndef __jsonar__
#define __jsonar__

#include <string>
#include <vector>
#include <map>
#include <exception>

namespace jsonar{
	enum class ValueType {
		VT_STR,
		VT_NUM,
		VT_OBJ,
		VT_ARR,
		VT_BOOL,
		VT_NULL,
		VT_NIL
	};
	class Object;
	class Array;
	class Value {
	public:
		Value(ValueType t) : type(t) {}
		ValueType getType() { return type; }
		virtual const char* asStr() const { return NULL; }
		virtual int asInt() { return 0; }
		virtual double asDouble() { return 0; }
		virtual Object* asObject() { return NULL; }
		virtual Array* asArray() { return NULL; }
		virtual bool asBool() { return false; }
		virtual bool isObject() { return false; }
		virtual bool isArray() { return false; }
		virtual bool isNull() { return false; }
	protected:

		ValueType type;
	};

	class String : public Value{
	public:
		String(const char* s) :data(s), Value(ValueType::VT_STR) {}
		const char* asStr() const { return data.c_str(); }
		bool operator< (const String& rhs) const {
			return data < rhs.data;
		}
	private:
		std::string data;
	};

	class Number : public Value {
	public:
		Number(double db) : d(db), Value(ValueType::VT_NUM) {}
		int asInt() { return static_cast<int>(d); }
		double asDouble() { return d; }
	private:
		double d;
	};

	class Object : public Value{
	public:
		Object() : Value(ValueType::VT_OBJ) {}
		void add(String &k, Value* v) { obj[k] = v; }
		Value* getValByKey(const char* key) { return obj[String(key)]; }
		std::map<String, Value*>& getVal() { return obj; }
		Object* asObject() { return this; }
		bool isObject() { return true; }
	private:
		std::map<String, Value*> obj;
	};

	class Array : public Value {
	public:
		Array() :Value(ValueType::VT_ARR) {}
		void push_back(Value* v) { vec.push_back(v); }
		Array* asArray() { return this; }
		bool isArray() { return true; }
		std::vector<Value*>& getVal(){ return vec; }
	private:
		std::vector<Value*> vec;
	};

	class Bool : public Value{
	public:
		Bool(bool _b) :b(_b), Value(ValueType::VT_BOOL) {}
		bool asBool() { return b; }
	private:
		bool b;
	};

	class Null : public Value{
	public:
		Null() :Value(ValueType::VT_NULL) {}
		bool isNull() { return true; }
	};

	class InvalidJson : public std::exception {
	public:
		InvalidJson(const char* e) : errmsg(e){}
		InvalidJson(const char* e, const char* json) : errmsg(e) { errmsg.append(" at :").append(json); }
		const char* what() const throw() { return errmsg.c_str(); }
	private:
		std::string errmsg;
	};

	class Json{
	public:
		Json(const char* json) :data(json) {}
		Json(std::string & s) :data(s){}
		~Json();
		Value* parse();
	private:
		void delValue(Value* v);
		void skipws();
		bool match(char c);
		bool matchWord(std::string w);
		bool matchString(std::string& s);
		bool matchObject(Object* obj);
		bool matchArray(Array* arr);
		bool matchValue(Value** value, bool canEmpty);
		bool matchPair(Object* obj, bool canEmpty);
		bool matchNum(double &d);
		bool lookhead(char c);
		bool isInSeqence(std::string seq);
		const char* getInfo();
	private:
		int pos = 0;
		std::string data;
		Value* value = nullptr;
	};

}

#endif /* defined(__jsonar__) */
