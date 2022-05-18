/*
 *  File: MyScope.cpp
 */

#include <cassert>

#include "MyScope.h"

MyScope::MyScope(MyScope *enclosed)
    : _enclose(enclosed)
{
}

/*
 *   Function: search
 *   Description: Will search for a particular name within a given
 *   Scope.
 */
MySymbol *MyScope::search(const string &name) const
{
    for (unsigned i = 0; i < _symbols.size(); ++i)
        if (name == _symbols[i]->name())
            return _symbols[i];

    return nullptr;
}

/*
 *   Function: lookfoor
 *   Description: Will search for a particular name within enclosing
 *   scopes recursively.
 */
MySymbol *MyScope::lookfor(const string &name) const
{
    MySymbol *sym;

    if ((sym = search(name)) != nullptr)
        return sym;

    return _enclose != nullptr
               ? _enclose->lookfor(name)
               : nullptr;
}

/*
 *   Function: insert
 *   Description: Given a symbol, will first check if the name has been entered
 *   within the scope then insert it into end of the vector table.
 */
void MyScope::insert(MySymbol *sym)
{
    assert(search(sym->name()) == nullptr);

    _symbols.push_back(sym);
}

/*
 *  Function: remove
 *  Description: Given a name, will check the given scope to
 *  find symbol with the corresponding name. When found
 *  earse it from the table.
 *
 */
void MyScope::remove(const string &name)
{
    for (unsigned i = 0; i < _symbols.size(); ++i)
    {
        if (name == _symbols[i]->name())
        {
            _symbols.erase(_symbols.begin() + i);
            break;
        }
    }
}

/*
 *   Function: enclose
 *   Description: Simply returns the enclosing scope.
 */
MyScope *MyScope::enclose() const
{
    return _enclose;
}

/*
 *   Function: symbols
 *   Description: Simply returns the Symbols vector table
 *   of a given scope
 */
const Symbols &MyScope::symbols() const
{
    return _symbols;
}