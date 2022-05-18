/*
 *
 * File: parser.cpp
 *
 * Description: This file contains the public and private functions and
 *       variable definitions for the parser analyzer for Simple C.
 *
 */

#include <iostream>
#include <unistd.h>
#include "tokens.h"
#include "string.h"
using namespace std;
#include "parser.h"
#include "lexer.h"
int lookAhead = 0;
string lexbuf;

int peekAhead = 0;
string peekbuf;

void debug()
{
	cout << "\tmatched:" << lexbuf << endl;
	lookAhead = lexan(lexbuf);
	cout << "next:" << lexbuf << endl;
	sleep(1);
}

void match(int t)
{

	if (lookAhead != t)
	{
		cout << "\tla token:" << lookAhead << endl;
		cout << "\ttoken:" << t << endl;
		report("Incorrect token found");
	}
	if (peekAhead != 0)
	{
		lookAhead = peekAhead;
		lexbuf = peekbuf;
		peekAhead = 0;
		peekbuf.clear();
	}
	else
	{
		// debug();
		lookAhead = lexan(lexbuf);
	}
}

int peek()
{
	if (peekAhead == 0)
		peekAhead = lexan(peekbuf);
	return peekAhead;
}

bool isSpecifier(int la)
{
	int check = spec();
	if (la == check)
	{
		return true;
	}
	else
	{
		cout << "check failed:" << check << endl;
		return false;
	}
}

void expression()
{
	logicAndExp();
	while (lookAhead == OR)
	{
		match(OR);
		logicAndExp();
		cout << "or" << endl;
	}
}

void logicAndExp()
{
	eqlExp();
	while (lookAhead == AND)
	{
		match(AND);
		eqlExp();
		cout << "and" << endl;
	}
}

void eqlExp()
{
	relateExp();
	while (
		lookAhead == EQL ||
		lookAhead == NEQ)
	{
		int op = lookAhead;
		match(lookAhead);
		relateExp();
		if (op == EQL)
			cout << "equal" << endl;
		else
			cout << "not equal" << endl;
	}
}

void relateExp()
{
	addExp();
	while (
		lookAhead == LTN ||
		lookAhead == GTN ||
		lookAhead == LEQ ||
		lookAhead == GEQ)
	{
		int op = lookAhead;
		match(lookAhead);
		addExp();
		if (op == LTN)
			cout << "less than" << endl;
		else if (op == GTN)
			cout << "greater than" << endl;
		else if (op == LEQ)
			cout << "less than or equal to" << endl;
		else
			cout << "greater than or equal to" << endl;
	}
}

void addExp()
{
	multExp();
	while (
		lookAhead == PLUS ||
		lookAhead == MINUS)
	{
		int op = lookAhead;
		match(lookAhead);
		multExp();
		if (op == PLUS)
			cout << "add" << endl;
		else
			cout << "sub" << endl;
	}
}

void multExp()
{
	preFixExp();
	while (
		lookAhead == STAR ||
		lookAhead == DIV ||
		lookAhead == REM)
	{
		int op = lookAhead;
		match(lookAhead);
		preFixExp();
		if (op == STAR)
			cout << "mul" << endl;
		else if (op == DIV)
			cout << "div" << endl;
		else
			cout << "rem" << endl;
	}
}

void preFixExp()
{
	if (lookAhead == NOT)
	{
		match(NOT);
		preFixExp();
		cout << "not" << endl;
	}
	else if (lookAhead == MINUS)
	{
		match(MINUS);
		preFixExp();
		cout << "neg" << endl;
	}
	else if (lookAhead == STAR)
	{
		match(STAR);
		preFixExp();
		cout << "deref" << endl;
	}
	else if (lookAhead == ADDR)
	{
		match(ADDR);
		preFixExp();
		cout << "addr" << endl;
	}
	else if (lookAhead == SIZEOF)
	{
		match(SIZEOF);
		if (
			peek() == INT ||
			peek() == CHAR ||
			peek() == STRUCT)
		{
			match('(');
			spec();
			ptr();
			match(')');
			cout << "sizeof" << endl;
		}
		else
		{
			preFixExp();
			cout << "sizeof" << endl;
		}
	}
	else if (lookAhead == '(')
	{
		if (
			peek() == INT ||
			peek() == CHAR ||
			peek() == STRUCT)
		{

			match('(');
			spec();
			ptr();
			match(')');
			preFixExp();
			cout << "cast" << endl;
		}
		else
		{
			postFixExp();
		}
	}
	else
		postFixExp();
}

void postFixExp()
{
	priExp();
	while (
		lookAhead == '[' ||
		lookAhead == '(' ||
		lookAhead == DOT ||
		lookAhead == ARROW)
	{
		int op = lookAhead;
		match(lookAhead);
		if (op == '[')
		{
			expression();
			match(']');
			cout << "index" << endl;
		}
		else if (op == '(')
		{
			if (lookAhead != ')')
				expressionList();
			match(')');
			cout << "call" << endl;
		}
		else if (op == DOT)
		{
			match(ID);
			cout << "dot" << endl;
		}
		else if (op == ARROW)
		{
			match(ID);
			cout << "arrow" << endl;
		}
	}
}

void priExp()
{
	if (lookAhead == '(')
	{
		match('(');
		expression();
		match(')');
	}
	else if (lookAhead == ID)
	{
		match(ID);
	}
	else if (lookAhead == CHAR)
	{
		match(CHAR);
	}
	else if (lookAhead == STRING)
	{
		match(STRING);
	}
	else if (lookAhead == NUM)
	{
		match(NUM);
	}
}

int spec()
{
	if (lookAhead == INT)
	{
		match(INT);
		return INT;
	}

	else if (lookAhead == CHAR)
	{
		match(CHAR);
		return CHAR;
	}
	else if (lookAhead == STRUCT)
	{
		match(STRUCT);
		match(ID);
		return STRUCT;
	}
	else
	{
		return -1;
	}
}

void ptr()
{
	if (lookAhead == STAR)
	{
		match(STAR);
		ptr();
	}
}

void expressionList()
{
	expression();
	while (1)
	{
		if (lookAhead == COMMA)
		{
			match(COMMA);
			expressionList();
		}
		else
		{
			break;
		}
	}
}

void parameter()
{
	spec();
	ptr();
	if (lookAhead == '(')
	{
		match('(');
		match('*');
		match(ID);
		match(')');
		match('(');
		match(')');
	}
	else
		match(ID);
}

void remainingDelcs()
{
	if (lookAhead == COMMA)
	{
		match(COMMA);
		glob_decl();
		remainingDelcs();
	}
	else if (lookAhead == SEMI)
	{
		match(SEMI);
	}
}

void parameter_list()
{
	parameter();
	if (lookAhead == COMMA)
	{
		match(',');
		parameter_list();
	}
}

void parameters()
{
	if (lookAhead == VOID)
	{
		match(VOID);
	}
	else
	{
		parameter_list();
	}
}

void declarator()
{
	ptr();
	if (lookAhead == LPAREN)
	{
		match('(');
		match('*');
		match(ID);
		match(')');
		match('(');
		match(')');
	}
	else
	{
		match(ID);
		if (lookAhead == LBRACK)
		{
			match('[');
			match(NUM);
			match(']');
		}
	}
}

void declaration()
{
	spec();
	declarator_list();
	match(';');
}

void declarations()
{
	if (
		lookAhead == INT ||
		lookAhead == CHAR ||
		lookAhead == STRUCT)
	{
		declaration();
		declarations();
	}
}

void declarator_list()
{
	declarator();
	if (lookAhead == COMMA)
	{
		match(',');
		declarator_list();
	}
}

void stmts()
{
	if (lookAhead != RBRACE)
	{
		stmt();
		stmts();
	}
}

void stmt()
{
	if (lookAhead == LBRACE)
	{
		match(LBRACE);
		declarations();
		stmts();
		match('}');
	}
	else if (lookAhead == RETURN)
	{
		match(RETURN);
		expression();
		match(';');
	}
	else if (lookAhead == FOR)
	{
		match(FOR);
		match('(');
		assignment();
		match(';');
		expression();
		match(';');
		assignment();
		match(')');
		stmt();
	}
	else if (
		lookAhead == WHILE ||
		lookAhead == IF)
	{
		int op = lookAhead;
		match(lookAhead);
		match('(');
		expression();
		match(')');
		stmt();
		if (op == IF)
		{
			if (lookAhead == ELSE)
			{
				match(ELSE);
				stmt();
			}
		}
	}
	else
	{
		// Assignment here:
		assignment();
		match(';');
	}
}

void glob_decl()
{
	ptr();
	if (lookAhead == '(')
	{
		match('(');
		match(STAR);
		match(ID);
		match(')');
		match('(');
		match(')');
	}
	else
	{
		match(ID);
		if (lookAhead == '(')
		{
			match('(');
			match(')');
		}
		else if (lookAhead == '[')
		{
			match('[');
			match(NUM);
			match(']');
		}
	}
}

void assignment()
{
	expression();
	while (lookAhead == ASSIGN)
	{
		match(ASSIGN);
		expression();
	}
}

void fun_or_glob()
{
	if (spec() == STRUCT && lookAhead == '{')
	{
		// This is the type-def:
		match('{');
		declarations();
		match('}');
		match(';');
	}
	else
	{
		if (lookAhead == '(')
		{
			// This is the function pointer:
			match('(');
			match('*');
			match(ID);
			match(')');
			match('(');
			match(')');
			remainingDelcs();
		}
		else
		{
			ptr();
			match(ID);
			if (lookAhead == '[')
			{
				match('[');
				match(NUM);
				match(']');
				remainingDelcs();
			}
			else if (lookAhead == '(')
			{
				match('(');

				if (lookAhead == ')')
				{
					match(')');
					remainingDelcs();
				}
				else
				{
					parameters();
					match(')');
					match('{');
					declarations();
					stmts();
					match('}');
				}
			}
			else
				remainingDelcs();
		}
	}
}

int main()
{
	lookAhead = lexan(lexbuf);
	while (lookAhead != DONE)
		fun_or_glob();

	exit(EXIT_SUCCESS);
}
