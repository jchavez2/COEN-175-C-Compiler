/*
 *   File: MySymbol.h
 */
#ifndef MYSYMBOL_H
#define MYSYMBOL_H

#include <string>
#include "Type.h"

class MySymbol
{

    typedef std::string string;

    Type _type;
    string _name;

public:
    bool defined;

    MySymbol(const string &name, const Type &type);

    const string &name() const;
    const Type &type() const;
};

#endif /* MYSYMBOL_H */