
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <stdio.h>
#include "lexer.h"
#include "checker.h"
#include "tokens.h"
#include "Type.h"
#include "MySymbol.h"
#include "MyScope.h"

using namespace std;

static bool debug = false;

/*
 *   These are for ouputing the semantic errors the
 *   checker finds.
 */
static string redefined = "redefinition of '%s'";
static string conflicting = "conflicting types for '%s'";
static string redeclared = "redeclaration of '%s'";
static string undeclared = "'%s' undeclared";
static string ptrRequired = "pointer type required for '%s'";
static string incomplete = "'%s' has incomplete type";

/*
 *  This is the global scope of the c file:
 */

static MyScope *global;

/*
 *   The (deepest) scope we are currently in:
 */
static MyScope *curScope;

/*
 *   Function: openScope
 *
 *   Description: Keeps track of opened scopes for our
 *   simple C complier.
 *
 */

MyScope *openScope()
{
    if (debug)
        cout << "OpenScope()" << endl;

    global = new MyScope(global);

    if (curScope == nullptr)
        curScope = global;

    return global;
}

/*
 *   Function: closedScope
 *
 *   Description: Keeps track of closed scopes for our
 *   simple C complier.
 *
 */
MyScope *closeScope()
{
    if (debug)
        cout << "closeScope()" << endl;

    MyScope *prev = global;
    global = global->enclose();

    return prev;
}

/*
 *   Function: declareFunc
 *
 *  Description: Indicates when a function is declared.
 */
MySymbol *declareFunc(const string &name, const Type &type)
{
    if (debug)
        cout << "functionDeclared(): " << name << endl;

    MySymbol *sym = curScope->search(name);

    if (sym == nullptr)
    {
        sym = new MySymbol(name, type);
        curScope->insert(sym);
    }
    else if (type != sym->type())
        report(conflicting, name); // ERROR [E2]
    else if (curScope != global)
        report(redeclared, name); // ERROR [E3]

    return sym;
}

/*
 *   Function: declareVariable
 *
 *  Description: Indicates when a variable is defined.
 */

MySymbol *declareVariable(const string &name, const Type &type)
{
    if (debug)
        cout << "declareVariable:" << name << endl;

    MySymbol *sym = global->search(name);

    if (curScope != global && type.specifier() == "struct" && type.indirection() == 0)
        report(ptrRequired, name); // ERROR [E6]

    if (sym == nullptr)
    {
        sym = new MySymbol(name, type);
        global->insert(sym);
    }
    else if (curScope != global)
        report(redeclared, name); //[E3]
    else if (type != sym->type())
        report(conflicting, name); //[E2]
    return sym;
}

/*
 *   Function: declareStruct
 *   Description: This is a declartion for a structure
 */
MySymbol *declareStruct(const string &name, const Type &ty, const string &structName)
{
    assert(ty.specifier() == "struct");

    if (debug)
        cout << "declareStruct: " << name << "|" << structName << endl;

    MySymbol *sym = curScope->search(name);

    /* Check if Specifier is a struct type, if so then ptrs must be non-empty [E5]*/
    if (ty.indirection() == 0)
        report(ptrRequired, name);
    else if (sym == nullptr)
    {
        sym = new MySymbol(name, ty);
        curScope->insert(sym);
    }
    else
        report(redefined, name); // ERROR [E3]

    return sym;
}

MySymbol *defineFunc(const std::string &name, const Type &type)
{
    if (debug)
        cout << "defineFunc():" << name << endl;

    MySymbol *sym = curScope->search(name);

    if (sym == nullptr)
        sym = declareFunc(name, type);
    else if (sym->defined)
        report(redefined, name); // ERROR [E1]
    sym->defined = true;
    return sym;
}

MySymbol *defineStruct(const string &name)
{
    if (debug)
        cout << "defineStruct:" << name << endl;

    MySymbol *sym = curScope->search(name);

    if (sym == nullptr)
    {
        sym = new MySymbol(name, Type(STRUCT));
        curScope->insert(sym);
    }
    else if (sym->defined)
    {
        report(redefined, name); // ERROR [E1]
    }
    sym->defined = true;

    return sym;
}

bool checkIfStructure(std::string &typespec)
{
    if (typespec != "int" || typespec != "char")
        return true;
    else
        return false;
}

MySymbol *checkStruct(const string &name, const string &structName)
{
    MySymbol *sym = global->lookfor(name);

    if (sym == nullptr)
        report(incomplete, structName); // ERROR [E6]
    return sym;
}

/*
 *   Function: checkID
 *
 *  Description: Checks to see if the ID is already within
 *  the scope, etc.
 */

MySymbol *checkID(const std::string name)
{
    if (debug)
        cout << "CheckID:" << name << endl;
    MySymbol *sym = global->lookfor(name);

    if (sym == nullptr)
    {
        report(undeclared, name);
        sym = new MySymbol(name, Type());
        global->insert(sym);
    }

    return sym;
}