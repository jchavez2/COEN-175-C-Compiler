/*
 *
 * File: parser.h
 *
 * Description: This file contains the public and variable decs for the
 * lex analzer for Simple C
 *
 */
#include <string>

void fun_or_glob();
void match(int t);
bool isSpecifier(int la);
void expression();
void logicAndExp();
void eqlExp();
void relateExp();
void addExp();
void multExp();
void preFixExp();
void postFixExp();
void priExp();
int spec();
void ptr();
void expressionList();
void remainingDelcs();
void parameters();
void declarations();
void declartion();
void declarator_list();
void declarator();
void stmt();
void stmts();
void glob_decl();
void assignment();
