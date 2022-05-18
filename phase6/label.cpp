/*
 *   File:   label.cpp
 *
 *   Description: The file contains the member functions
 *   for labels provided in the assembly
 *
 */

#include <iostream>
#include "label.h"
using namespace std;

unsigned Label::_counter = 0;

Label::Label()
{
    _number = _counter++;
}

unsigned Label::number() const
{
    return _number;
}

ostream &operator<<(ostream &ostr, const Label &label)
{
    return ostr << ".L" << label.number();
}