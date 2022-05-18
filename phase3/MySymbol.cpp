/*
 *   File: MySymbol.cpp
 */

#include "MySymbol.h"

using std::string;

typedef std::string string;

MySymbol::MySymbol(const string &name, const Type &type) : _name(name), _type(type), defined(false)
{
}
/*
 *   Function: name
 *   Description: Simply return the name of a given symbol
 */
const string &MySymbol::name() const
{
    return _name;
}

/*
 *  Function: type
 *  Description: Simply returns the associated type information
 *  of a given symbol.
 */
const Type &MySymbol::type() const
{
    return _type;
}
