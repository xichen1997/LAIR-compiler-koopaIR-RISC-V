#pragma once
#include <iostream>
#include <memory>
#include <string>
#include "koopa.h"

using namespace std;

extern int temp_count;

class BaseAST;
class CompUnit;
class FuncDef;
class FuncType;
class Block;
class Stmt;
class Exp;
// Expression Classes
class UnaryExp;
class PrimaryExp;
class Number;
class MulExp;
class AddExp;
class RelExp;
class EqExp;
class LAndExp;
class LOrExp;


class BaseAST{
    public:
    int lineno;
    virtual ~BaseAST() = default;

    virtual void Dump() = 0;
    virtual void GenerateIR() = 0;
};

class CompUnit : public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() override;
    void GenerateIR() override;
};
// varName used for generating RISCV code.
// other used for generating AST koopa IR.

class FuncDef : public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_type;
    std::unique_ptr<string> ident;
    std::unique_ptr<BaseAST> block;

    void Dump() override;
    void GenerateIR() override;
};

class FuncType : public BaseAST{
    public:
    std::unique_ptr<string> functype;
    void Dump() override;
    void GenerateIR() override;
};

class Block : public BaseAST{
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() override;
    void GenerateIR() override;
};


class Number : public BaseAST{
    public:
    int int_const;

    void Dump() override;
    void GenerateIR() override;
};

class Exp : public BaseAST{
    public:
    std::unique_ptr<string> varName;
    std::unique_ptr<LOrExp> lor_exp;

    void Dump() override;
    void GenerateIR() override;
};

class PrimaryExp : public BaseAST{
    public:
    enum Kind{
        _Exp,
        _Number
    }kind;

    std::unique_ptr<string> varName;
    std::unique_ptr<Exp> exp;
    std::unique_ptr<Number> number;

    void Dump() override;
    void GenerateIR() override;
};

class UnaryExp : public BaseAST{
    public:
    enum Kind{
        _PrimaryExp,
        _UnaryOp_UnaryExp
    } kind;
    
    std::unique_ptr<string> varName;
    std::unique_ptr<PrimaryExp> primary_exp;
    std::unique_ptr<string> unary_op;
    std::unique_ptr<UnaryExp> unary_exp;

    void Dump() override;
    
    void GenerateIR() override;
};

class Stmt : public BaseAST{
    public:
    std::unique_ptr<Exp> exp;

    void Dump() override;
    void GenerateIR() override;
};


class MulExp : public BaseAST{
    public:
    enum Kind { _UnaryExp, _MulExp_MulOp_UnaryExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<MulExp> mul_exp;
    std::unique_ptr<string> mul_op;
    std::unique_ptr<UnaryExp> unary_exp;

    void Dump() override;
    void GenerateIR() override;
};

class AddExp : public BaseAST{
    public:
    enum Kind { _MulExp, _AddExp_AddOp_MulExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<AddExp> add_exp;
    std::unique_ptr<string> add_op;
    std::unique_ptr<MulExp> mul_exp;

    void Dump() override;
    void GenerateIR() override;
};

class RelExp : public BaseAST{
    public:
    enum Kind { _AddExp, _RelExp_RelOp_AddExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<RelExp> rel_exp;
    std::unique_ptr<string> rel_op;
    std::unique_ptr<AddExp> add_exp;

    void Dump() override;
    void GenerateIR() override;
};

class EqExp : public BaseAST{
    public:
    enum Kind { _RelExp, _EqExp_EqOp_RelExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<EqExp> eq_exp;
    std::unique_ptr<string> eq_op;
    std::unique_ptr<RelExp> rel_exp;

    void Dump() override;
    void GenerateIR() override;
};

class LAndExp : public BaseAST{
    public:
    enum Kind { _EqExp, _LAndExp_LAndOp_EqExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<LAndExp> land_exp;
    std::unique_ptr<string> land_op;
    std::unique_ptr<EqExp> eq_exp;

    void Dump() override;
    void GenerateIR() override;
};

class LOrExp : public BaseAST{
    public:
    enum Kind { _LAndExp, _LOrExp_LOrOp_LAndExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<LOrExp> lor_exp;
    std::unique_ptr<string> lor_op;
    std::unique_ptr<LAndExp> land_exp;

    void Dump() override;
    void GenerateIR() override;
};