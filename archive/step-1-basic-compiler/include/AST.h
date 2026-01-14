#pragma once
#include <iostream>

using namespace std;


class BaseAST{
    public:
    virtual ~BaseAST() = default; // Virtual destructor for base class, it's necessary.

    virtual void Dump() const = 0;
    virtual void GenerateIR() const = 0;
};

class CompUnit : public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override{
        cout << "CompUnitAST { ";
        func_def->Dump();
        cout << " }";
    }
    void GenerateIR() const override{
        // Implementation for IR generation would go here
        func_def->GenerateIR();
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
        cout << *ident << " , ";
        
        block->Dump();
        cout << " }";
    }
    void GenerateIR() const override{
        // Implementation for IR generation would go here
        cout << "fun @" << *ident << "(): "; 
        func_type->GenerateIR();
        block->GenerateIR();
    }
};

class FuncType : public BaseAST{
    public:
    std::unique_ptr<string> functype;
    void Dump() const override{
        cout << "FuncType: { " << *functype << " }";
    }
    void GenerateIR() const override{
        // Implementation for IR generation would go here
        if(functype->compare("int") == 0){
            cout << "i32 ";
        }
    }
};

class Block : public BaseAST{
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override{
        cout << "Block: { ";
        stmt->Dump();
        cout << " }";
    }
    void GenerateIR() const override{
        // Implementation for IR generation would go here
        cout << "{\n";
        cout << "\%entry:\n";
        stmt->GenerateIR();
        cout << "\n}\n";
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
    void GenerateIR() const override{
        // Implementation for IR generation would go here
        cout << "  ret ";
        number->GenerateIR();
    }
};

class Number : public BaseAST{
    public:
    int int_const;

    void Dump() const override{
        cout << to_string(int_const);
    }
    void GenerateIR() const override{
        // Implementation for IR generation would go here
        cout << to_string(int_const);
    }
};