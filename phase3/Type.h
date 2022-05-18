/*
 *  File: Type.h
 *
 */
#ifndef TYPE_H
#define TYPE_H

#include <ostream>
#include <string>
#include <vector>
#include <cassert>

enum kind
{
    ARRAY,
    CALLBACK,
    ERROR,
    FUNCTION,
    SCALAR
};

typedef std::vector<class Type> Parameters;

class Type
{
    typedef std::string string;
    int _kind;
    string _specifier;
    unsigned _indirection;

    unsigned _length;
    Parameters *_parameters;

public:
    Type(int kind);
    Type(int kind, const string &specifier, unsigned indirection);
    Type(int kind, const string &specifier, unsigned indirection, unsigned length);
    Type(int kind, const string &specifier, unsigned indirection, Parameters *paras);
    Type();

    const string &specifier() const { return _specifier; }
    unsigned indirection() const { return _indirection; }

    bool isArray() const { return _kind == ARRAY; }
    bool isCallback() const { return _kind == CALLBACK; }
    bool isError() const { return _kind == ERROR; }
    bool isFunction() const { return _kind == FUNCTION; }
    bool isScalar() const { return _kind == SCALAR; }

    bool operator==(const Type &rhs) const;
    bool operator!=(const Type &rhs) const;

    bool isStruct() const;

    unsigned length() const
    {
        assert(_kind == ARRAY);
        return _length;
    }
    Parameters *parameters() const
    {
        assert(_kind == FUNCTION);
        return _parameters;
    }
    void parameters(Parameters *params) { _parameters = params; }
};

std::ostream &operator<<(std::ostream &ostr, const Type &type);

#endif /* TYPE_H */