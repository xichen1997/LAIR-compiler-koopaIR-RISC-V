#include "../include/AST.h"

using namespace std;

int temp_count = 0;

void CompUnit::Dump() {
    cout << "CompUnitAST { ";
    func_def->Dump();
    cout << " }";
}
void CompUnit::GenerateIR() {
    // Implementation for IR generation would go here
    func_def->GenerateIR();
}

void FuncDef::Dump() {
    cout << "FuncDef { ";
    func_type->Dump();

    cout << " , ";
    cout << *ident << " , ";
    
    block->Dump();
    cout << " }";
}

void FuncDef::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "fun @" << *ident << "(): "; 
    func_type->GenerateIR();
    block->GenerateIR();
}

void FuncType::Dump() {
    cout << "FuncType: { " << *functype << " }";
}

void FuncType::GenerateIR() {
    // Implementation for IR generation would go here
    if(functype->compare("int") == 0){
        cout << "i32 ";
    }
}

void Block::Dump() {
    cout << "Block: { ";
    stmt->Dump();
    cout << " }";
}

void Block::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "{\n";
    cout << "\%entry:\n";
    stmt->GenerateIR();
    cout << "\n}\n";
}

void Stmt::Dump() {
    cout << "return ";
    exp->Dump();
    cout << ";";
}

void Stmt::GenerateIR() {
    // Implementation for IR generation would go here
    exp->GenerateIR();
    cout << "  ret ";
    cout << *(exp->varName) << "\n";
}

void Number::Dump() {
    cout << to_string(int_const);
}

void Number::GenerateIR() {
    // Implementation for IR generation would go here
    // cout << to_string(int_const);
}

void Exp::Dump() {
    cout << "Exp: { ";
    unary_exp->Dump();
    cout << " }";
}

void Exp::GenerateIR() {
    // Implementation for IR generation would go here
    unary_exp->GenerateIR();
    varName = std::make_unique<string>(*(unary_exp->varName));
}

void PrimaryExp::Dump() {
    cout << "PrimaryExp: { ";
    if(kind == _Exp){
        exp->Dump();
    } else if(kind == _Number){
        number->Dump();
    }
    cout << " }";
}

void PrimaryExp::GenerateIR(){
    // Implementation for IR generation would go here
    if(kind == _Exp){
        exp->GenerateIR();
        // get the pointer and dereference to get the varName value from Exp
        varName = std::make_unique<string>(*(exp->varName));
    } else if(kind == _Number){
        // number->GenerateIR();
        varName = std::make_unique<string>(to_string(number->int_const));  
    }
}

void UnaryExp::Dump() {
    cout << "UnaryExp: { ";
    if(kind == _PrimaryExp){
        primary_exp->Dump();
    } else if(kind == _UnaryOp_UnaryExp){
        cout << *unary_op << " ";
        unary_exp->Dump();
    }
    cout << " }";
}

void UnaryExp::GenerateIR(){
    // Implementation for IR generation would go here
    if(kind == _PrimaryExp){
        primary_exp->GenerateIR();
        varName = std::make_unique<string>(*(primary_exp->varName));
    } else if(kind == _UnaryOp_UnaryExp){
        unary_exp->GenerateIR();
        varName = std::make_unique<string>("\%" + to_string(temp_count++));
        cout << "  " << *varName << " = ";
        if(unary_op->compare("+") == 0){
            // do nothing because +x is just x
        }else if(unary_op->compare("-") == 0){
            cout << "sub 0, " << *(unary_exp->varName) << "\n";
        } else if(unary_op->compare("!") == 0){
            cout << "eq " << *(unary_exp->varName) << ", 0\n";
        }
    }
}

void UnaryOp::Dump() {
    cout << "UnaryOp: { " << *unary_op << " }";
}

void UnaryOp::GenerateIR() {
    // do nothing
}

