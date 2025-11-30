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
class UnaryExp;
class PrimaryExp;
class Number;


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
    std::unique_ptr<UnaryExp> unary_exp;

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

class UnaryOp : public BaseAST{
    public:
    std::unique_ptr<string> unary_op;

    void Dump() override;
    void GenerateIR() override;
}; 


class Stmt : public BaseAST{
    public:
    std::unique_ptr<Exp> exp;

    void Dump() override;
    void GenerateIR() override;
};
