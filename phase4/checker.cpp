/*
 * File:	checker.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the semantic checker for Simple C.
 *
 *		Extra functionality:
 *		- inserting an undeclared symbol with the error type
 */

#include <map>
#include <set>
#include <iostream>
#include "lexer.h"
#include "checker.h"
#include "Symbol.h"
#include "Scope.h"
#include "Type.h"

using namespace std;

static set<string> functions;
static map<string, Scope *> fields;
static Scope *outermost, *toplevel;
static const Type error;
static Scalar integer("int");

static string undeclared = "'%s' undeclared";
static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string incomplete = "'%s' has incomplete type";
static string nonpointer = "pointer type required for '%s'";

static string invalidReturn = "invalid return type";
static string invalidTest = "invalid type for test expression";
static string requiredLval = "lvalue required in expression";
static string invalidBinary = "invalid operands to binary %s";
static string invalidUnary = "invalid operand to unary %s";
static string invalidCast = "invalid operand in cast expression";
static string callFun = "called object is not a function";
static string invalidArgs = "invalid arguments to called function";
static string incompleteType = "using pointer to incomplete type";

static bool debug = false;

#define isStructure(t) (t.isStruct() && t.indirection() == 0)

/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope *
openScope()
{
    toplevel = new Scope(toplevel);

    if (outermost == nullptr)
        outermost = toplevel;

    return toplevel;
}

/*
 * Function:	closeScope
 *
 * Description:	Remove the top-level scope, and make its enclosing scope
 *		the new top-level scope.
 */

Scope *closeScope()
{
    Scope *old = toplevel;

    toplevel = toplevel->enclosing();
    return old;
}

/*
 * Function:	openStruct
 *
 * Description:	Open a scope for a structure with the specified name.  If a
 *		structure with the same name is already defined, delete it.
 */

void openStruct(const string &name)
{
    if (fields.count(name) > 0)
    {
        delete fields[name];
        fields.erase(name);
        report(redefined, name);
    }

    openScope();
}

/*
 * Function:	closeStruct
 *
 * Description:	Close the scope for the structure with the specified name.
 */

void closeStruct(const string &name)
{
    fields[name] = closeScope();
}

/*
 * Function:	declareSymbol
 *
 * Description:	Declare a symbol with the specified NAME and TYPE.  Any
 *		erroneous redeclaration is discarded.  If a declaration has
 *		multiple errors, only the first error is reported.  To
 *		report multiple errors, remove the "return" statements and,
 *		if desired, the final "else".
 */

void declareSymbol(const string &name, const Type &type, bool isParameter)
{
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr)
        toplevel->insert(new Symbol(name, type));
    else if (toplevel != outermost)
    {
        report(redeclared, name);
        return;
    }
    else if (type != symbol->type())
    {
        report(conflicting, name);
        return;
    }

    if (isStructure(type))
    {
        if (isParameter || type.isCallback() || type.isFunction())
            report(nonpointer, name);
        else if (fields.count(type.specifier()) == 0)
            report(incomplete, name);
    }
}

/*
 * Function:	defineFunction
 *
 * Description:	Define a function with the specified NAME and TYPE.  A
 *		function is always defined in the outermost scope.  This
 *		definition always replaces any previous definition or
 *		declaration.  In the case of multiple errors, only the
 *		first error is reported.  To report multiple errors, remove
 *		the "else"s.
 */

Symbol *defineFunction(const string &name, const Type &type)
{
    Symbol *symbol = outermost->find(name);

    if (functions.count(name) > 0)
        report(redefined, name);
    else if (symbol != nullptr && type != symbol->type())
        report(conflicting, name);
    else if (isStructure(type))
        report(nonpointer, name);

    outermost->remove(name);
    delete symbol;

    symbol = new Symbol(name, type);
    outermost->insert(symbol);

    functions.insert(name);
    return symbol;
}

/*
 * Function:	checkIdentifier
 *
 * Description:	Check if NAME is declared.  If it is undeclared, then
 *		declare it as having the error type in order to eliminate
 *		future error messages.
 */

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr)
    {
        report(undeclared, name);
        symbol = new Symbol(name, error);
        toplevel->insert(symbol);
    }

    return symbol;
}
// True if the type is a struct and the structure is not compelete.
// static bool checkIsCompleteStruct(const Type &t)
// {
//     return (t.isStruct() && fields.count(t.specifier()) == 0);
// }

// A pointer is complete if it refers to a struct that has previosuly been defined.
static bool isCompletePointer(const Type t)
{
    return !(t.isStruct() && t.indirection() == 1 && fields.count(t.specifier()) == 0);
}
/*
 *   Function: checkLogicalOr
 *   Description: Returns a check to see if the operands of logical Or check
 *   is of a value type.
 */

Type checkLogicalOr(const Type &left, const Type &right)
{
    if (debug)
        cout << "check ||" << endl;

    if (left.isError() || right.isError())
        return error;

    if (left.isValue() && right.isValue())
        return integer;

    report(invalidBinary, "||");
    return error;
}

Type checkLogicalAnd(const Type &left, const Type &right)
{
    if (debug)
        cout << "check &&" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isValue() && right.isValue())
        return integer;

    report(invalidBinary, "&&");
    return error;
}

Type checkEquality(const Type &left, const Type &right)
{
    if (debug)
        cout << "check ==" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isCompatibleWith(right))
        return integer;

    report(invalidBinary, "==");
    return error;
}

Type checkInequality(const Type &left, const Type &right)
{
    if (debug)
        cout << "check !=" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isCompatibleWith(right))
        return integer;

    report(invalidBinary, "!=");
    return error;
}
Type checkLessThan(const Type &left, const Type &right)
{
    if (debug)
        cout << "check <" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isCompatibleWith(right))
        return integer;

    report(invalidBinary, "<");

    return error;
}
Type checkGreaterThan(const Type &left, const Type &right)
{
    if (debug)
        cout << "check >" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isCompatibleWith(right))
        return integer;

    report(invalidBinary, ">");
    return error;
}
Type checkLessOrEqual(const Type &left, const Type &right)
{
    if (debug)
        cout << "check <=" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isCompatibleWith(right))
        return integer;

    report(invalidBinary, "<=");
    return error;
}

Type checkGreaterOrEqual(const Type &left, const Type &right)
{
    if (debug)
        cout << "chcek >=" << endl;
    if (left.isError() || right.isError())
        return error;
    if (left.isCompatibleWith(right))
        return integer;

    report(invalidBinary, ">=");
    return error;
}

Type checkAddition(const Type &left, const Type &right)
{
    if (debug)
        cout << "check +" << endl;
    if (left.isError() || right.isError())
        return error;
    Type t1 = left.promote();
    Type t2 = right.promote();

    if (t1.isInteger() && t2.isInteger())
        return integer;
    if (t1.isPointer() && t2.isInteger())
    {
        // Check if the struct is complete
        if (isCompletePointer(t1))
        {
            return t1;
        }
        report(incompleteType);
        return error;
    }
    if (t1.isValue() && t2.isPointer())
    {
        if (isCompletePointer(t2))
        {
            return t2;
        }
        //[E9]
        report(incompleteType);
        return error;
    }
    //[E4]
    report(invalidBinary, "+");
    return error;
}

Type checkSubtraction(const Type &left, const Type &right)
{
    bool innerDebug = false;
    if (debug)
        cout << "check -" << endl;
    if (left.isError() || right.isError())
        return error;
    Type t1 = left.promote();
    Type t2 = right.promote();
    if (t1.isInteger() && t2.isInteger())
        return integer;
    if (t1.isPointer() && t2.isInteger())
    {
        if (!isCompletePointer(t1))
        {
            report(incompleteType);
            return error;
        }

        return t1;
    }

    if (innerDebug)
    {
        cout << "left spec:" << t1.specifier() << endl;
        cout << "right spec:" << t2.specifier() << endl;
    }

    if ((t1.isPointer() && t2.isPointer()) && (t1.specifier() == t2.specifier()))
    {
        if (isCompletePointer(t1) && isCompletePointer(t2))
        {
            if (t1.isCompatibleWith(t2))
                return integer;
            //[E4]
            report(invalidBinary, "-");
            return error;
        }
        report(incompleteType);
        return error;
    }
    report(invalidBinary, "-");
    return error;
}

Type checkMultiply(const Type &left, const Type &right)
{
    if (debug)
        cout << "check * (mult)" << endl;
    if (left.isError() || right.isError())
        return error;
    Type t1 = left.promote();
    Type t2 = right.promote();

    if (t1.isInteger() && t2.isInteger())
        return integer;
    report(invalidBinary, "*");
    return error;
}
Type checkDivision(const Type &left, const Type &right)
{
    if (debug)
        cout << "check /" << endl;
    if (left.isError() || right.isError())
        return error;
    Type t1 = left.promote();
    Type t2 = right.promote();

    if (t1.isInteger() && t2.isInteger())
        return integer;
    report(invalidBinary, "*");
    return error;
}

Type checkPercent(const Type &left, const Type &right)
{
    if (debug)
        cout << "check %" << endl;
    if (left.isError() || right.isError())
        return error;
    Type t1 = left.promote();
    Type t2 = right.promote();

    if (t1.isInteger() && t2.isInteger())
        return integer;
    report(invalidBinary, "*");
    return error;
}

Type checkNot(const Type &expr)
{
    if (debug)
        cout << "check ! (prefix)" << endl;
    if (expr.isError())
        return error;

    if (!expr.isValue())
    {
        report(invalidUnary, "!");
        return error;
    }

    return Scalar("int");
}

Type checkNeg(const Type &expr)
{
    if (debug)
        cout << "check - (prefix)" << endl;
    if (expr.isError())
        return error;
    Type t = expr.promote();

    if (t.isInteger())
    {
        return Scalar("int");
    }

    report(invalidUnary, "-");
    return error;
}

Type checkDeref(const Type &expr)
{
    if (debug)
        cout << "check * (deref)" << endl;
    if (expr.isError())
        return error;
    Type t = expr.promote();
    if (!t.isPointer())
    {
        //[E5]
        report(invalidUnary, "*");
        return error;
    }

    if (isCompletePointer(t))
        return Scalar(t.specifier());

    //[E9]
    report(incompleteType);
    return error;
}

Type checkAddress(const Type &expr, const bool &lvalue)
{
    if (debug)
        cout << "check & (prefix)" << endl;
    if (expr.isError())
        return error;

    if (!lvalue)
    {
        report(requiredLval); //[E3]
        return error;
    }

    if (expr.isCallback())
    {
        report(invalidUnary, "&"); //[E5]
        return error;
    }

    return Scalar(expr.specifier(), expr.indirection() + 1);
}

Type checkSizeof(const Type &expr)
{
    if (debug)
        cout << "check sizeof (prefix)" << endl;
    if (expr.isError())
        return error;
    if (expr.isFunction())
    {
        report(invalidUnary, "sizeof");
        return error;
    }
    if (!isCompletePointer(expr))
    {
        report(incompleteType);
        return error;
    }

    return integer;
}

Type checkTypeCast(const Type &left, const Type &right)
{
    if (debug)
        cout << "check Cast" << endl;
    if (left.isError() || right.isError())
        return error;

    Type t1 = left.promote();
    Type t2 = right.promote();

    if (t1.isInteger() && t2.isInteger())
    {
        return t1;
    }

    if (t1.isPointer() && t2.isPointer())
    {
        if (isCompletePointer(t1) && isCompletePointer(t2))
            return t1;
    }

    report(invalidCast);
    return error;
}

Type checkArray(const Type &left, const Type &right)
{
    bool innerDebug = false;
    if (left.isError() || right.isError())
        return error;

    Type t1 = left.promote();
    // Type t2 = right.promote();
    if (innerDebug)
    {
        cout << "left is:" << left.specifier() << endl;
        cout << "right is:" << right.specifier() << endl;
        cout << "left indirection:" << left.indirection() << endl;
        cout << "right indirection:" << right.indirection() << endl;
    }

    if (t1.isPointer())
    {
        if (right.isInteger())
        {
            if (t1.isStruct() && (fields.count(t1.specifier()) == 0))
            {
                //[E9]
                report(incompleteType);
                return error;
            }
            return Scalar(t1.specifier(), t1.indirection() - 1);
        }
        //[E4]
        report(invalidBinary, "[]");
        return error;
    }
    //[E4]
    report(invalidBinary, "[]");
    return error;
}

Type checkDirectStrcutField(const Type &left, const string id)
{
    if (left.isError())
        return error;

    if (left.isStruct())
    {
        if (fields.find(left.specifier()) != fields.end())
        {

            const Symbols typeFields = fields.find(left.specifier())->second->symbols();
            for (unsigned i = 0; i < typeFields.size(); ++i)
                if (typeFields[i]->name() == id)
                    return typeFields[i]->type();

            //[E4]
            report(invalidBinary, ".");
            return error;
        }
        //[E4]
        report(invalidBinary, ".");
        return error;
    }
    //[E4]
    report(invalidBinary, ".");
    return error;
}

Type checkIndirectStructure(const Type &left, const string id)
{
    if (left.isError())
        return error;

    const Type t1 = left.promote();

    if (t1.isPointer() && t1.isStruct())
    {

        if (isCompletePointer(t1))
        {
            if (fields.find(t1.specifier()) != fields.end())
            {
                const Symbols typeFields = fields.find(t1.specifier())->second->symbols();
                for (unsigned i = 0; i < typeFields.size(); ++i)
                    if (typeFields[i]->name() == id)
                        return typeFields[i]->type();
            }
            //[E4]
            report(invalidBinary, "->");
            return error;
        }
        //[E9]
        report(incompleteType);
        return error;
    }
    //[E4]
    report(invalidBinary, "->");
    return error;
}

Type checkCall(const Type &left, Parameters &args)
{
    Type ty1;
    Type ty2;
    if (left.isError())
    {
        return error;
    }

    if (!(left.isFunction() || left.isCallback()))
    {
        //[E7]
        report(callFun);
        return error;
    }

    for (auto t : args)
    {
        t.promote();
        if (!t.isValue())
        {
            report(invalidArgs);
            return error;
        }
    }

    Parameters *formals = left.parameters();

    if (formals != nullptr)
    {

        if (formals->size() != args.size())
        {
            report(invalidArgs);
            return error;
        }

        for (unsigned i = 0; i < args.size(); i++)
        {
            ty1 = (*formals)[i];
            ty2 = args[i];

            if (!ty1.isCompatibleWith(ty2))
            {
                //[E8]
                report(invalidArgs);
                return error;
            }
        }
    }

    return Scalar(left.specifier(), left.indirection());
}

Type checkReturn(const Type &expr, const Type &type)
{
    if (expr.isError() || type.isError())
        return error;

    if (expr.isCompatibleWith(type))
        return expr;

    report(invalidReturn);
    return error;
}

Type checkConditional(const Type &expr)
{
    if (expr.isError())
        return error;

    if (expr.isValue())
        return expr;

    report(invalidTest);
    return error;
}

Type checkAssignment(const Type &left, const Type &right, const bool &lvalue)
{
    bool innerDebug = false;
    if (left.isError() || right.isError())
        return error;

    if (innerDebug)
    {
        cout << "\tleft is:" << left.specifier() << endl;
        cout << "\tright is:" << right.specifier() << endl;
        cout << "left indirection:" << left.indirection() << endl;
        cout << "right indirection:" << right.indirection() << endl;
        cout << (left.promote() == right.promote()) << endl;
    }

    if (lvalue)
    {
        if (left.isCompatibleWith(right))
            return left;
        // [E4]
        report(invalidBinary, "=");
        return error;
    }

    //[E3]
    report(requiredLval);
    return error;
}