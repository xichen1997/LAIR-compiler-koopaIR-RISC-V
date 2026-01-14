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
    add_exp->Dump();
    cout << " }";
}

void Exp::GenerateIR() {
    // Implementation for IR generation would go here
    add_exp->GenerateIR();
    varName = std::make_unique<string>(*(add_exp->varName));
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


void MulOp::Dump() {
    cout << "MulOp: { " << *mul_op << " }";
}

void MulOp::GenerateIR() {
    // do nothing
}

void AddOp::Dump() {
    cout << "AddOp: { " << *add_op << " }";
}
void AddOp::GenerateIR() {
    // do nothing
}

void MulExp::Dump() {
    cout << "MulExp: { ";
    unary_exp->Dump();
    cout << " }";
}

void MulExp::GenerateIR() {
    // Implementation for IR generation would go here
    if(kind == _UnaryExp){
        unary_exp->GenerateIR();
        varName = make_unique<string>(*(unary_exp->varName));
    }
    else if(kind == _MulExp_MulOp_UnaryExp){
        mul_exp->GenerateIR();
        unary_exp->GenerateIR();

        string leftVar = *(mul_exp->varName);
        string rightVar = *(unary_exp->varName);
        varName = make_unique<string>("\%" + to_string(temp_count++));
        
        if(mul_op->compare("*") == 0){
            cout << "  " << *varName << " = mul " << leftVar << ", " << rightVar << "\n";
        } else if(mul_op->compare("/") == 0){
            cout << "  " << *varName << " = div " << leftVar << ", " << rightVar << "\n";
        } else if(mul_op->compare("%") == 0){
            cout << "  " << *varName << " = rem " << leftVar << ", " << rightVar << "\n";
        }
    }
}

void AddExp::Dump() {
    cout << "AddExp: { ";
    mul_exp->Dump();
    cout << " }";
}

void AddExp::GenerateIR(){
    if(kind == _MulExp){
        mul_exp->GenerateIR();
        varName = make_unique<string>(*(mul_exp->varName));
    }else if(kind == _AddExp_AddOp_MulExp){
        add_exp->GenerateIR();
        mul_exp->GenerateIR();

        string leftVar = *(add_exp->varName);
        string rightVar = *(mul_exp->varName);
        varName = make_unique<string>("\%" + to_string(temp_count++));
        
        if(add_op->compare("+") == 0){
            cout << "  " << *varName << " = add " << leftVar << ", " << rightVar << "\n";
        } else if(add_op->compare("-") == 0){
            cout << "  " << *varName << " = sub " << leftVar << ", " << rightVar << "\n";
        }
    }
}