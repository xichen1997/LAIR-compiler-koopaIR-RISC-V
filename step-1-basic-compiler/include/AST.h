#pragma once
#include <iostream>

using namespace std;


class BaseAST{
    public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

class CompUnit : public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override{
        cout << "CompUnitAST { ";
        func_def->Dump();
        cout << " }";
    }
};

class FuncDef : public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_type;
    std::unique_ptr<string> ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override{
        cout << "FuncDef { ";
        func_type->Dump();

        cout << " , ";
        cout << "ident: " << *ident << " , ";
        
        block->Dump();
        cout << " }";
    }
};

class FuncType : public BaseAST{
    public:
    std::unique_ptr<string> functype;
    void Dump() const override{
        cout << "functype: " << *functype;
    }
};

class Block : public BaseAST{
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override{
        cout << "block: { ";
        stmt->Dump();
        cout << " }";
    }
};

class Stmt : public BaseAST{
    public:
    std::unique_ptr<BaseAST> number;

    void Dump() const override{
        cout << "return ";
        number->Dump();
        cout << ";";
    }
};

class Number : public BaseAST{
    public:
    int int_const;

    void Dump() const override{
        cout << to_string(int_const);
    }
};