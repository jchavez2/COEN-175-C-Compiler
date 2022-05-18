/*
 * File:	checker.h
 *
 * Description:	This file contains the public function declarations for the
 *		semantic checker for Simple C.
 */

#ifndef CHECKER_H
#define CHECKER_H
#include <string>
#include "Scope.h"

Scope *openScope();
Scope *closeScope();

void openStruct(const std::string &name);
void closeStruct(const std::string &name);

void declareSymbol(const std::string &name, const Type &type, bool = false);

Symbol *defineFunction(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);

Type checkLogicalOr(const Type &left, const Type &right);
Type checkLogicalAnd(const Type &left, const Type &right);
Type checkEquality(const Type &left, const Type &right);
Type checkInequality(const Type &left, const Type &right);
Type checkLessThan(const Type &left, const Type &right);
Type checkGreaterThan(const Type &left, const Type &right);
Type checkLessOrEqual(const Type &left, const Type &right);
Type checkGreaterOrEqual(const Type &left, const Type &right);
Type checkAddition(const Type &left, const Type &right);
Type checkSubtraction(const Type &left, const Type &right);
Type checkMultiply(const Type &left, const Type &right);
Type checkDivision(const Type &left, const Type &right);
Type checkPercent(const Type &left, const Type &right);
Type checkNot(const Type &expr);
Type checkNeg(const Type &expr);
Type checkDeref(const Type &expr);
Type checkAddress(const Type &expr, const bool &lvalue);
Type checkSizeof(const Type &expr);
Type checkTypeCast(const Type &left, const Type &right);
Type checkArray(const Type &left, const Type &right);
Type checkDirectStrcutField(const Type &left, const std::string id);
Type checkIndirectStructure(const Type &left, const std::string id);
Type checkCall(const Type &left, Parameters &args);
Type checkReturn(const Type &expr, const Type &type);
Type checkConditional(const Type &expr);
Type checkAssignment(const Type &left, const Type &right, const bool &lvalue);

#endif /* CHECKER_H */
