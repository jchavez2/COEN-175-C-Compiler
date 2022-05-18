

#ifndef CHECKER_H
#define CHECKER_H
#include <string>

#include "MyScope.h"
#include "MySymbol.h"

MyScope *openScope();
MyScope *closeScope();
MySymbol *declareFunc(const std::string &name, const Type &type);
MySymbol *defineFunc(const std::string &name, const Type &type);
MySymbol *declareVariable(const std::string &name, const Type &type);
bool checkIfStructure(std::string &typespec);
MySymbol *checkID(const std::string name);
MySymbol *declareStruct(const std::string &name, const Type &ty, const std::string &structName);
MySymbol *defineStruct(const std::string &name);
MySymbol *checkStruct(const std::string &name, const std::string &structName);

#endif /* CHECK_H */