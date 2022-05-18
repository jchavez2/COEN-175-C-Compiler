/*
 * File:	generator.cpp
 *
 * Description:	This file contains the public and member function
 *		definitions for the code generator for Simple C.
 *
 *		Extra functionality:
 *		- putting all the global declarations at the end
 */

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include "generator.h"
#include "machine.h"
#include "Tree.h"

static void compute(Expression *result, Expression *left, Expression *right, const std::string &opcode);
static void divide(Expression *result, Expression *left, Expression *right, Register *reg);
static void compare(Expression *result, Expression *left, Expression *right, const std::string &opcode);
static void findBaseAndOffset(Expression *expr, Expression *&base, int &offset);

using namespace std;

bool debug = false;

static int offset;
static string funcname;
static ostream &operator<<(ostream &ostr, Expression *expr);

static Register *eax = new Register("%eax", "%al");
static Register *ecx = new Register("%ecx", "%cl");
static Register *edx = new Register("%edx", "%dl");

static map<string, Label> strings;
static vector<Register *> registers = {eax, ecx, edx};

/* These will be replaced with functions in the next phase.  They are here
   as placeholders so that Call::generate() is finished. */

// #define assign(node, reg)
// #define load(node, reg)

/*
 * Function:	align (private)
 *
 * Description:	Return the number of bytes necessary to align the given
 *		offset on the stack.
 */

static int align(int offset)
{
    if (offset % STACK_ALIGNMENT == 0)
        return 0;

    return STACK_ALIGNMENT - (abs(offset) % STACK_ALIGNMENT);
}

/*
 * Function:	operator << (private)
 *
 * Description:	Convenience function for writing the operand of an
 *		expression using the output stream operator.
 */

static ostream &operator<<(ostream &ostr, Expression *expr)
{
    if (expr->_register != nullptr)
        return ostr << expr->_register;

    expr->operand(ostr);
    return ostr;
}

/*
 * Function:	Expression::operand
 *
 * Description:	Write an expression as an operand to the specified stream.
 */

void Expression::operand(ostream &ostr) const
{
    assert(_offset != 0);
    ostr << _offset << "(%ebp)";
}

/*
 * Function:	Identifier::operand
 *
 * Description:	Write an identifier as an operand to the specified stream.
 */

void Identifier::operand(ostream &ostr) const
{
    if (_symbol->_offset == 0)
        ostr << global_prefix << _symbol->name();
    else
        ostr << _symbol->_offset << "(%ebp)";
}

/*
 * Function:	Number::operand
 *
 * Description:	Write a number as an operand to the specified stream.
 */

void Number::operand(ostream &ostr) const
{
    ostr << "$" << _value;
}

/*
 * Function:	Call::generate
 *
 * Description:	Generate code for a function call expression.
 *
 * 		On a 32-bit Linux platform, the stack needs to be aligned
 * 		on a 4-byte boundary.  (Online documentation seems to
 * 		suggest that 16 bytes are required, but 4 bytes seems to
 * 		work and is much easier.)  Since all arguments are 4-bytes
 *		wide, the stack will always be aligned.
 *
 *		On a 32-bit OS X platform, the stack needs to aligned on a
 *		16-byte boundary.  So, if the stack will not be aligned
 *		after pushing the arguments, we first adjust the stack
 *		pointer.  However, this trick only works if none of the
 *		arguments are themselves function calls.
 *
 *		To handle nested function calls, we need to generate code
 *		for the nested calls first, which requires us to save their
 *		results and then push them on the stack later.  For
 *		efficiency, we only first generate code for the nested
 *		calls, but generate code for ordinary arguments in place.
 */

void Call::generate()
{
    unsigned numBytes;

    /* Generate code for any nested function calls first. */

    numBytes = 0;

    for (int i = _args.size() - 1; i >= 0; i--)
    {
        numBytes += _args[i]->type().size();

        if (STACK_ALIGNMENT != SIZEOF_REG && _args[i]->_hasCall)
            _args[i]->generate();
    }

    /* Align the stack if necessary. */

    if (align(numBytes) != 0)
    {
        cout << "\tsubl\t$" << align(numBytes) << ", %esp" << endl;
        numBytes += align(numBytes);
    }

    /* Generate code for any remaining arguments and push them on the stack. */

    for (int i = _args.size() - 1; i >= 0; i--)
    {
        if (STACK_ALIGNMENT == SIZEOF_REG || !_args[i]->_hasCall)
            _args[i]->generate();

        cout << "\tpushl\t" << _args[i] << endl;
        assign(_args[i], nullptr);
    }

    /* Call the function and then reclaim the stack space. */

    load(nullptr, eax);
    load(nullptr, ecx);
    load(nullptr, edx);

    if (_expr->type().isCallback())
    {
        _expr->generate();

        if (_expr->_register == nullptr)
            load(_expr, getreg());

        cout << "\tcall\t*" << _expr << endl;
        assign(_expr, nullptr);
    }
    else
        cout << "\tcall\t" << _expr << endl;

    if (numBytes > 0)
        cout << "\taddl\t$" << numBytes << ", %esp" << endl;

    assign(this, eax);
}

/*
 * Function:	Block::generate
 *
 * Description:	Generate code for this block, which simply means we
 *		generate code for each statement within the block.
 */

void Block::generate()
{
    for (auto stmt : _stmts)
    {
        stmt->generate();

        for (auto reg : registers)
            assert(reg->_node == nullptr);
    }
}

/*
 * Function:	Simple::generate
 *
 * Description:	Generate code for a simple (expression) statement, which
 *		means simply generating code for the expression.
 */

void Simple::generate()
{
    _expr->generate();
    assign(_expr, nullptr);
}

/*
 * Function:	Procedure::generate
 *
 * Description:	Generate code for this function, which entails allocating
 *		space for local variables, then emitting our prologue, the
 *		body of the function, and the epilogue.
 */

void Procedure::generate()
{
    int param_offset;

    /* Assign offsets to the parameters and local variables. */

    param_offset = 2 * SIZEOF_REG;
    offset = param_offset;
    allocate(offset);

    /* Generate our prologue. */

    funcname = _id->name();
    cout << global_prefix << funcname << ":" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << funcname << ".size, %esp" << endl;

    /* Generate the body of this function. */

    _body->generate();

    /* Generate our epilogue. */

    cout << endl
         << global_prefix << funcname << ".exit:" << endl;
    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl
         << endl;

    offset -= align(offset - param_offset);
    cout << "\t.set\t" << funcname << ".size, " << -offset << endl;
    cout << "\t.globl\t" << global_prefix << funcname << endl
         << endl;
}

/*
 * Function:	generateGlobals
 *
 * Description:	Generate code for any global variable declarations.
 */

void generateGlobals(Scope *scope)
{
    const Symbols &symbols = scope->symbols();

    for (auto symbol : symbols)
        if (!symbol->type().isFunction())
        {
            cout << "\t.comm\t" << global_prefix << symbol->name() << ", ";
            cout << symbol->type().size() << endl;
        }

    cout << "\t.data" << endl;
    for (pair<string, Label> string_p : strings)
    {
        cout << string_p.second << ":\t.asciz\t\"";
        for (auto &ch : string_p.first)
        {
            if (ch != '\n')
            {
                cout << ch;
            }
            else
                cout << "\\n";
        }
        cout << "\"" << endl;
    }
}

/*
 * Function:	Assignment::generate
 *
 * Description:	Generate code for an assignment statement.
 *
 *		NOT FINISHED: Only works if the right-hand side is an
 *		integer literal and the left-hand side is an integer
 *		scalar.
 */

void Assignment::generate()
{
    if (debug)
        cout << "# ASSIGNMENT::GENERATE" << endl;
    // assert(dynamic_cast<Number *>(_right));
    // assert(dynamic_cast<Identifier *>(_left));
    Expression *base;
    Expression *ptr;
    findBaseAndOffset(_left, base, offset);

    unsigned num;
    _right->generate();

    if (!_right->isNumber(num) || _right->_register == nullptr)
    {
        load(_right, getreg());
    }

    if (base->isDereference(ptr))
    {
        ptr->generate();

        if (ptr->_register == nullptr)
        {
            load(ptr, getreg());
        }
        unsigned n = _right->type().size();
        if (n == 1)
        {
            cout << "\tmovb\t" << _right->_register->name(n) << ", "
                 << "(" << ptr << ")" << endl;
        }
        else
        {
            cout << "\tmovl\t" << _right << ", "
                 << "(" << ptr << ")" << endl;
        }
        assign(ptr, nullptr);
    }
    else
    {

        base->generate();
        unsigned n = _right->type().size();
        if (n == 1)
        {
            cout << "\tmovb\t"
                 << _right->_register->name(n)
                 << ", " << offset << "+" << base << endl;
        }
        else
        {
            cout << "\tmovl\t" << _right << ", " << offset << "+" << base << endl;
        }
        // assign(base, nullptr);
    }
    assign(_right, nullptr);
}

/*
 *   Function: load
 *   load an expression into a given register.
 *
 */

void load(Expression *expr, Register *reg)
{
    if (debug)
        cout << "# LOAD" << endl;
    if (reg->_node != expr)
    {
        if (reg->_node != nullptr)
        {
            unsigned n = reg->_node->type().size();
            offset -= n;
            reg->_node->_offset = offset;
            cout << (n == 1 ? "\tmovb\t" : "\tmovl\t");
            cout << reg << ", " << offset << "(%ebp)" << endl;
        }
        if (expr != nullptr)
        {
            unsigned n = expr->type().size();
            cout << (n == 1 ? "\tmovb\t" : "\tmovl\t");
            cout << expr << ", " << reg->name(n) << endl;
        }
        assign(expr, reg);
    }
}

/*
 *   Function: getReg
 *   Returns the first aviable register as a ptr.
 *
 */

Register *getreg()
{
    for (auto reg : registers)
        if (reg->_node == nullptr)
            return reg;

    load(nullptr, registers[0]);
    return registers[0];
}

/*
 *   Function: assign
 *   Simply assigns an expression to a register (No policy decisions are made).
 */

void assign(Expression *expr, Register *reg)
{
    if (debug)
        cout << "# ASSIGN Reg" << endl;
    if (expr != nullptr)
    {
        if (expr->_register != nullptr)
        {
            expr->_register->_node = nullptr;
        }
        expr->_register = reg;
    }

    if (reg != nullptr)
    {
        if (reg->_node != nullptr)
        {
            reg->_node->_register = nullptr;
        }
        reg->_node = expr;
    }
}

static void compute(Expression *result, Expression *left, Expression *right, const string &opcode)
{
    if (debug)
        cout << "# compute HERE:" << endl;
    left->generate();
    right->generate();

    if (left->_register == nullptr)
    {
        load(left, getreg());
    }

    cout << "\t" << opcode << "\t" << right << ", " << left << endl;

    assign(right, nullptr);
    assign(result, left->_register);
}

void Add::generate()
{
    if (debug)
        cout << "# ADD::GENERATE" << endl;
    compute(this, _left, _right, "addl");
}

void Subtract::generate()
{
    if (debug)
        cout << "# SUB::GENERATE" << endl;
    compute(this, _left, _right, "subl");
}

void Multiply::generate()
{
    if (debug)
        cout << "# MULT::GENERATE" << endl;
    compute(this, _left, _right, "imull");
}

static void divide(Expression *result, Expression *left, Expression *right, Register *reg)
{
    unsigned int num;
    left->generate();
    right->generate();
    load(left, eax);
    load(nullptr, edx);
    if (right->isNumber(num))
    {
        load(right, ecx);
    }

    cout << "\tcltd\t" << endl;
    cout << "\tidivl\t"
         << right << endl;

    assign(nullptr, left->_register);
    assign(nullptr, right->_register);
    assign(result, reg);
}

void Divide::generate()
{
    divide(this, _left, _right, eax);
}

void Remainder::generate()
{
    divide(this, _left, _right, edx);
}

static void compare(Expression *result, Expression *left, Expression *right, const string &opcode)
{
    left->generate();
    right->generate();
    if (left->_register == nullptr)
        load(left, getreg());
    cout << "\tcmpl\t" << right << ", " << left << endl;
    cout << "\t" << opcode << "\t" << left->_register->byte() << endl;
    cout << "\tmovzbl\t" << left->_register->byte() << ", " << left->_register << endl;

    assign(result, left->_register);
}

void LessThan::generate()
{
    compare(this, _left, _right, "setl");
}

void GreaterThan::generate()
{
    compare(this, _left, _right, "setg");
}

void LessOrEqual::generate()
{
    compare(this, _left, _right, "setle");
}

void GreaterOrEqual::generate()
{
    compare(this, _left, _right, "setge");
}

void Equal::generate()
{
    if (debug)
        cout << "# EQUAL TO" << endl;
    compare(this, _left, _right, "sete");
}

void NotEqual::generate()
{
    compare(this, _left, _right, "setne");
}

void Cast::generate()
{
    _expr->generate();
    if (_expr->_register == nullptr)
    {
        load(_expr, getreg());
    }

    if (this->type().size() == 4)
    {
        if (_expr->type().size() == 1)
        {

            cout << "\tmovsbl\t" << _expr << ", " << _expr->_register->name(4) << endl;
        }
    }

    assign(this, _expr->_register);
}

void Not::generate()
{
    _expr->generate();
    if (_expr->_register == nullptr)
    {
        load(_expr, getreg());
    }

    cout << "\tcmpl\t"
         << "$0"
         << ", " << _expr << endl;
    cout << "\tsete\t" << _expr->_register->byte() << endl;
    cout << "\tmovzbl\t" << _expr->_register->byte() << ", " << _expr << endl;

    assign(this, _expr->_register);
}

void Negate::generate()
{
    _expr->generate();
    if (_expr->_register == nullptr)
    {
        load(_expr, getreg());
    }

    cout << "\tnegl\t" << _expr << endl;

    assign(this, _expr->_register);
}

void Dereference::generate()
{
    _expr->generate();
    if (_expr->_register == nullptr)
    {
        load(_expr, getreg());
    }

    if (_expr->type().size() == 4)
    {
        cout << "\tmovl\t"
             << "(" << _expr << "), " << _expr << endl;
    }
    else
    {
        cout << "\tmovzbl\t"
             << "(" << _expr << "), " << _expr << endl;
    }
    assign(this, _expr->_register);
}

void Address::generate()
{
    Expression *base;
    findBaseAndOffset(_expr, base, offset);

    Expression *ptr;
    if (base->isDereference(ptr))
    {
        ptr->generate();
        if (ptr->_register == nullptr)
        {
            load(ptr, getreg());
        }
        assign(this, ptr->_register);
    }
    else
    {
        assign(this, getreg());
        cout << "\tleal\t" << base << ", " << this << endl;
    }
}

void String::operand(ostream &ostr) const
{
    Label string;

    if (strings.find(_value) == strings.end())
    {
        strings.insert({_value, string});
    }
    else
    {
        string = strings.find(_value)->second;
    }
    ostr << string;
}

void Expression::test(const Label &label, bool ifTrue)
{
    generate();

    if (_register == nullptr)
        load(this, getreg());

    cout << "\tcmpl\t$0, " << this << endl;
    cout << (ifTrue ? "\tjne\t" : "\tje\t") << label << endl;

    assign(this, nullptr);
}

static void findBaseAndOffset(Expression *expr, Expression *&base, int &offset)
{
    int field;

    base = expr;
    offset = 0;

    while (base->isField(base, field))
        offset += field;
}

void Field::generate()
{
    Expression *base;
    int offset = _id->_offset;
    findBaseAndOffset(_expr, base, offset);

    Expression *ptr;
    if (base->isDereference(ptr))
    {
        ptr->generate();
        if (ptr->_register == nullptr)
        {
            load(ptr, getreg());
        }
        assign(this, getreg());
        if (this->type().size() == 4)
        {
            cout << "\tmovl\t" << ptr << ", " << offset << "+(" << this << ")" << endl;
        }
        else
        {
            cout << "\tmovb\t" << ptr << ", " << offset << "+(" << this << ")" << endl;
        }
    }
    else
    {
        assign(_expr, getreg());
        if (this->type().size() == 4)
        {
            cout << "\tmovl\t" << base << ", " << offset << "+" << this << endl;
        }
        else
        {
            cout << "\tmovb\t" << base << ", " << offset << "+" << this << endl;
        }
    }
}

void LogicalAnd::generate()
{
    Label next;
    _left->test(next, false);

    _right->generate();
    if (_right->_register == nullptr)
    {
        load(_right, getreg());
    }
    cout << "\tcmpl\t$0, " << _right << endl;

    cout << next << ":" << endl;
    cout << "\tsetne\t" << _right->_register->byte() << endl;
    cout << "\tmovzbl\t" << _right->_register->byte() << ", " << _right << endl;
    assign(this, _right->_register);
}

void LogicalOr::generate()
{
    Label next;
    _left->test(next, true);

    _right->generate();
    if (_right->_register == nullptr)
    {
        load(_right, getreg());
    }

    cout << "\tcmpl\t$0, " << _right << endl;

    cout << next << ":" << endl;
    cout << "\tsetne\t" << _right->_register->byte() << endl;
    cout << "\tmovzbl\t" << _right->_register->byte() << ", " << _right << endl;
    assign(this, _right->_register);
}

void While::generate()
{
    Label loop, exit;

    cout << loop << ":" << endl;

    _expr->test(exit, false);
    _stmt->generate();

    cout << "\tjmp\t" << loop << endl;
    cout << exit << ":" << endl;
}

void LessThan::test(const Label &label, bool ifTrue)
{
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, getreg());

    cout << "\tcmpl\t" << _right << ", " << _left << endl;
    cout << (ifTrue ? "\tjl\t" : "\tjge\t") << label << endl;

    assign(_left, nullptr);
    assign(_right, nullptr);
}

void Return::generate()
{
    if (debug)
    {
        cout << "# RETURN GENERATE!" << endl;
    }
    _expr->generate();
    load(_expr, eax);
    cout << "\tjmp\t" << funcname << ".exit" << endl;
    assign(_expr, nullptr);
}

void For::generate()
{
    Label next, exit;
    _init->generate();
    cout << next << ":" << endl;

    _expr->test(exit, false);
    _stmt->generate();
    _incr->generate();

    cout << "\tjmp\t" << next << endl;
    cout << exit << ":" << endl;
}

void If::generate()
{
    Label next, exit;

    _expr->test(next, false);
    _thenStmt->generate();

    if (_elseStmt != nullptr)
    {

        cout << "\tjmp\t" << exit << endl;
        cout << next << ":" << endl;
        _elseStmt->generate();
        cout << exit << ":" << endl;
    }
    else
    {
        cout << next << ":" << endl;
    }
}
