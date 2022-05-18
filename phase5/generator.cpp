#include "generator.h"
#include <iostream>
#include <vector>

using namespace std;

typedef std::vector<Symbol *> LocalVars;

static ostream &operator<<(ostream &ostr, Expression *expr)
{
    expr->operand(ostr);
    return ostr;
}

void Procedure::generate()
{

    int offset = 0;
    int paramOffset = 8;
    unsigned numParams = _id->type().parameters()->size();
    LocalVars localVars = _body->declarations()->symbols();

    for (unsigned i = 0; i < localVars.size(); i++)
    {
        if (i < numParams)
        {
            localVars[i]->offset = paramOffset;
            paramOffset += localVars[i]->type().size();
        }
        else
        {
            offset += localVars[i]->type().size();
            localVars[i]->offset = -offset;
        }
    }

    cerr << "Params offset: " << paramOffset << endl;
    cerr << "Variables offset: " << offset << endl;

    cout << _id->name() << ":" << endl;
    cout << "# function prologue" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, ";
    cout << "%ebp" << endl;
    cout << "\tsubl\t$" << offset << ", %esp" << endl;

    _body->generate();

    cout << "# function epilogue:" << endl;
    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl;
    cout << "\t.globl\t" << _id->name() << endl;
}

void Block::generate()
{
    cout << "# block:" << endl;
    for (auto stmt : _stmts)
    {
        // cout << "# HERE LIES GENERATE" << endl;
        stmt->generate();
    }
}

void Assignment::generate()
{
    cout << "# assignment:" << endl;
    _right->generate();
    _left->generate();
    cout << "\tmovl\t" << _right << ", %eax" << endl;
    cout << "\tmovl\t%eax, " << _left << endl;
}

void Call::generate()
{
    cout << "# function call:" << endl;
    // Push the args from --> to <-- first
    for (int i = _args.size() - 1; i >= 0; i--)
    {
        _args[i]->generate();
        cout << "\tpushl\t" << _args[i] << endl;
    }
    cout << "\tcall\t" << _expr << endl;
}

void Simple::generate()
{

    _expr->generate();
}

void Number::operand(ostream &ostr) const
{
    cout << "$" << _value;
}

void Identifier::operand(ostream &ostr) const
{
    // IF it's a global, refer to it by name:
    if (_symbol->offset == 0)
        cout << _symbol->name();
    else
        cout << _symbol->offset << "(%ebp)";
}

void Expression::operand(ostream &ostr) const
{
}

void generateGlobals(const Symbols &global_syms)
{
    cout << "# global variables:" << endl;

    unsigned global_size = global_syms.size();
    for (unsigned i = 0; i < global_size; i++)
    {
        if (!global_syms[i]->type().isFunction())
        {
            cout << "\t.comm\t" << global_syms[i]->name();
            cout << ", " << global_syms[i]->type().size() << endl;
        }
    }
}