/*
 *
 */

#ifndef MYSCOPE_H
#define MYSCOPE_H

#include <vector>
#include "MySymbol.h"

typedef std::vector<MySymbol *> Symbols;

class MyScope
{
    typedef std::string string;

    Symbols _symbols;
    MyScope *_enclose;

public:
    MyScope(MyScope *enclosed = nullptr);

    MySymbol *search(const string &name) const;
    MySymbol *lookfor(const string &name) const;

    void remove(const string &name);
    void insert(MySymbol *sym);

    const Symbols &symbols() const;
    MyScope *enclose() const;
};

#endif /* MYSCOPE_H */