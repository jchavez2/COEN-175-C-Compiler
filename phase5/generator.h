#ifndef GENERATOR_H
#define GENERATOR_H

#include "Tree.h"
#include "lexer.h"
#include "Scope.h"
// #include "Type.h"
// #include "checker.h"

void generateGlobals(const Symbols &global_vars);

#endif