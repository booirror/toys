
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef char bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
typedef struct{
	union
	{
		float fval;
		int   ival;
	}v;
	bool isfloat;
} number_t;

static char buffer[1024];
static int pcurr = 0;
static int blen = 0;
static char token;

number_t exp();
number_t term();
number_t factor();
number_t value();
number_t number();
number_t calculate(char *str);
void calc_and_print(char* str);

int cc_main()
{
	calc_and_print("1+2+3*2*4");
	calc_and_print("2+3 * 4 + 4/2");
	calc_and_print("-(3+ 4) / 2.5");
	calc_and_print("-(3 * 4) + 2");
	calc_and_print("3.3 - 3");
	
	getchar();
	return 0;
}

void setbuff(char* buff)
{
	strcpy(buffer, buff);
	pcurr = 0;
	blen = strlen(buff);
}

void skipws()
{
	while (pcurr < blen) {
		if (buffer[pcurr] == ' ' || buffer[pcurr] == '/t' || buffer[pcurr] == '/v') {
			pcurr++;
		}
		else {
			break;
		}
	}
}

int getc()
{
	skipws();
	if (pcurr < blen) {
		return buffer[pcurr++];
	}
	else {
		return -1;
	}
}

void unget()
{
	pcurr--;
}

char* format(char* fmt, ...)
{
	static char buff[1024];
	va_list *args;
	va_start(args, fmt);
	vsprintf(buff, fmt, args);
	va_end(args);
	return buff;
}

void error(char *msg)
{
	fprintf(stderr, "ERROR: %s", msg);
	getchar();
	exit(1);
}
char* info(int match)
{
	static char infomsg[1024];
	if (pcurr < 2) {
		infomsg[0] = '\0';
		return infomsg;
	}
	char pos[1024];
	for (int i = 0; i < pcurr - 2; i++) {
		pos[i] = ' ';
	}
	pos[pcurr - 1] = '^';
	pos[pcurr] = '\0';
	vsprintf(infomsg, "position: %d, match %c failed ¡£\n%s\n%s", pcurr - 1, match, buffer, pos);
	return infomsg;
}
bool match(char c)
{
	if (token == c) {
		token = getc();
		return true;
	}
	else {
		error(info(c));
		return false;
	}
}

number_t calc(char op, number_t lval, number_t rval)
{
	number_t result;
	result.isfloat = lval.isfloat || rval.isfloat;
	switch (op)
	{
	case '+':
		if (result.isfloat) {
			result.v.fval = (lval.isfloat ? lval.v.fval : lval.v.ival) + (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		else {
			result.v.ival = (lval.isfloat ? lval.v.fval : lval.v.ival) + (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		break;
	case '-':
		if (result.isfloat) {
			result.v.fval = (lval.isfloat ? lval.v.fval : lval.v.ival) - (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		else {
			result.v.ival = (lval.isfloat ? lval.v.fval : lval.v.ival) - (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		break;
	case '*':
		if (result.isfloat) {
			result.v.fval = (lval.isfloat ? lval.v.fval : lval.v.ival) * (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		else {
			result.v.ival = (lval.isfloat ? lval.v.fval : lval.v.ival) * (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		break;
	case '/':
		if (result.isfloat) {
			result.v.fval = (lval.isfloat ? lval.v.fval : lval.v.ival) / (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		else {
			result.v.ival = (lval.isfloat ? lval.v.fval : lval.v.ival) / (rval.isfloat ? rval.v.fval : rval.v.ival);
		}
		break;
	default:
		break;
	}
	return result;
}

number_t exp()
{
	number_t num = term();
	number_t temp;
	while (token == '+' || token == '-') {
		switch (token)
		{
		case '+':
			match('+');
			temp = term();
			num = calc('+', num, temp);
			break;
		case '-':
			match('-');
			temp = term();
			num = calc('-', num, temp);
			break;
		default:
			break;
		}
	}
	return num;
}

number_t term()
{
	number_t num = factor();
	number_t temp;
	while (token == '*' || token == '/') {
		switch (token)
		{
		case '*':
			match('*');
			temp = term();
			num = calc('*', num, temp);
			break;
		case '/':
			match('/');
			temp = term();
			bool iszero = temp.isfloat ? temp.v.fval == 0 : temp.v.ival == 0;
			if (iszero) {
				error("divisor can't be 0");
			}
			num = calc('/', num, temp);
			break;
		default:
			break;
		}
	}
	return num;
}

number_t factor()
{
	number_t num;
	bool negtive = false;
	if (token == '-') {
		match('-');
		negtive = true;
	}
	num = value();
	if (negtive) {
		if (num.isfloat) {
			num.v.fval = 0 - num.v.fval;
		}
		else {
			num.v.ival = 0 - num.v.ival;
		}
	}
	return num;
}

bool isint(char *str)
{
	while (*str != '\0') {
		if (isdigit(*str)) {
			str++;
		}
		else {
			return false;
		}
	}
	return true;
}

bool isfloat(char *str)
{
	int dot = 0;
	while (*str != '\0') {
		if (isdigit(*str)) {
			str++;
		}
		else if (*str == '.') {
			str++;
			dot++;
		}
		else {
			return false;
		}
	}
	if (dot != 1) {
		return false;
	}
	return true;
}
number_t value()
{
	number_t num;
	if (token == '(') {
		match('(');
		num = exp();
		match(')');
	}
	else {
		num = number();
	}
	return num;
}

void scannum(char* dst)
{
	while (isdigit(buffer[pcurr]) || buffer[pcurr] == '.') {
		*dst++ = buffer[pcurr];
		pcurr++;
	}
	*dst = '\0';
}

number_t number()
{
	number_t num;
	if (isdigit(token) || token == '.') {
		char buff[128];
		unget();
		scannum(buff);
		if (isint(buff)) {
			num.v.ival = atoi(buff);
			num.isfloat = false;
		}
		else if (isfloat(buff)) {
			num.v.fval = atof(buff);
			num.isfloat = true;
		}
		else {
			error(info(' '));
		}
		token = getc();
	}
	else {
		error("exception a number");
	}
	return num;
}

number_t calculate(char *str)
{
	setbuff(str);
	token = getc();
	number_t num = exp();
	return num;
}

void calc_and_print(char* str)
{
	number_t num = calculate(str);
	if (num.isfloat) {
		printf("%s \nresult: %f\n", str, num.v.fval);
	}
	else {
		printf("%s \nresult: %d\n", str, num.v.ival);
	}
}