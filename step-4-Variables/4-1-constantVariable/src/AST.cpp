#include "../include/AST.h"
#include <cassert>

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

void Decl::Dump() {
    cout << "Decl: { ";
    const_decl->Dump();
    cout << " }";
}

void Decl::GenerateIR(){
    // Implementation for IR generation would go here
    const_decl->GenerateIR();
}

void ConstDecl::Dump() {
    cout << "ConstDecl: { ";
    const_def_list->Dump();
    cout << " }";
}

void ConstDecl::GenerateIR(){
    // Implementation for IR generation would go here
    const_def_list->GenerateIR();
}

void ConstDefList::Dump() {
    cout << "ConstDefList: { ";
    for(auto& const_def : const_defs){
        const_def->Dump();
    }
    cout << " }";
}

void ConstDefList::GenerateIR(){
    // Implementation for IR generation would go here
    for(auto& const_def : const_defs){
        const_def->GenerateIR();
    }
} 

void ConstDef::Dump() {
    cout << "ConstDef: { ";
    const_init_val->Dump();
    cout << " }";
}

void ConstDef::GenerateIR(){
    // Implementation for IR generation would go here
    const_init_val->GenerateIR();
}

void ConstInitVal::Dump() {
    cout << "ConstInitVal: { ";
    const_exp->Dump();
    cout << " }";
}

void ConstInitVal::GenerateIR(){
    // Implementation for IR generation would go here
    const_exp->GenerateIR();
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
    block_item_list->Dump();
    cout << " }";
}

void Block::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "{\n";
    cout << "\%entry:\n";
    block_item_list->GenerateIR();
    cout << "\n}\n";
}

void BlockItemList::Dump() {
    for(auto& block_item : block_items){
        block_item->Dump();
    }
}

void BlockItemList::GenerateIR() {
    for(auto& block_item : block_items){
        block_item->GenerateIR();
    }
}

void BlockItem::Dump() {
    if(kind == _Decl){
        decl->Dump();
    } else if(kind == _Stmt){
        stmt->Dump();
    }
}

void BlockItem::GenerateIR() {
    if(kind == _Decl){
        decl->GenerateIR();
    } else if(kind == _Stmt){
        stmt->GenerateIR();
    }
}

void Stmt::Dump() {
    if(kind == _Lval_Assign_Exp){
        cout << "Stmt: { Lval = ";
        cout << *lval << " , Exp = ";
        exp->Dump();
        cout << " }";
    } else if(kind == _Return_Exp){
        cout << "Stmt: { Return Exp = ";
        exp->Dump();
        cout << " }";
    }
}

void Stmt::GenerateIR() {
    if(kind == _Lval_Assign_Exp){
        exp->GenerateIR();
        cout << "  store " << *(exp->varName) << ", @" << *lval << "\n";
    } else if(kind == _Return_Exp){
        exp->GenerateIR();
        cout << "  ret " << *(exp->varName) << "\n";
    }
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
    lor_exp->Dump();
    cout << " }";
}

void Exp::GenerateIR() {
    // Implementation for IR generation would go here
    lor_exp->GenerateIR();
    varName = std::make_unique<string>(*(lor_exp->varName));
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
    } else if(kind == _Lval){
        // for Lval, just use the ident as varName
        varName = std::make_unique<string>(*ident);
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
            cout << "add 0, " << *(unary_exp->varName) << "\n";
        }else if(unary_op->compare("-") == 0){
            cout << "sub 0, " << *(unary_exp->varName) << "\n";
        } else if(unary_op->compare("!") == 0){
            cout << "eq " << *(unary_exp->varName) << ", 0\n";
        }
    }
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
            cout << "  " << *varName << " = mod " << leftVar << ", " << rightVar << "\n";
        } else{
            assert(false);
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
        } else{
            assert(false);
        }
    }
}

void RelExp::Dump() {
    cout << "RelExp: { ";
    add_exp->Dump();
    cout << " }";
}

void RelExp::GenerateIR(){
    if(kind == _AddExp){
        add_exp->GenerateIR();
        varName = make_unique<string>(*(add_exp->varName));
    }else if(kind == _RelExp_RelOp_AddExp){
        rel_exp->GenerateIR();
        add_exp->GenerateIR();

        string leftVar = *(rel_exp->varName);
        string rightVar = *(add_exp->varName);
        varName = make_unique<string>("\%" + to_string(temp_count++));
        
        if(rel_op->compare("<") == 0){
            cout << "  " << *varName << " = lt " << leftVar << ", " << rightVar << "\n";
        } else if(rel_op->compare(">") == 0){
            cout << "  " << *varName << " = gt " << leftVar << ", " << rightVar << "\n";
        } else if(rel_op->compare("<=") == 0){
            cout << "  " << *varName << " = le " << leftVar << ", " << rightVar << "\n";
        } else if(rel_op->compare(">=") == 0){
            cout << "  " << *varName << " = ge " << leftVar << ", " << rightVar << "\n";
        } else{
            assert(false);
        }
    }
}

void EqExp::Dump() {
    cout << "EqExp: { ";
    rel_exp->Dump();
    cout << " }";
}

void EqExp::GenerateIR(){
    if(kind == _RelExp){
        rel_exp->GenerateIR();
        varName = make_unique<string>(*(rel_exp->varName));
    }else if(kind == _EqExp_EqOp_RelExp){
        eq_exp->GenerateIR();
        rel_exp->GenerateIR();

        string leftVar = *(eq_exp->varName);
        string rightVar = *(rel_exp->varName);
        varName = make_unique<string>("\%" + to_string(temp_count++));
        
        if(eq_op->compare("==") == 0){
            cout << "  " << *varName << " = eq " << leftVar << ", " << rightVar << "\n";
        } else if(eq_op->compare("!=") == 0){
            cout << "  " << *varName << " = ne " << leftVar << ", " << rightVar << "\n";
        } else{
            assert(false);
        }
    }
}


void LAndExp::Dump() {
    cout << "LAndExp: { ";
    eq_exp->Dump();
    cout << " }";
}

void LAndExp::GenerateIR(){
    if(kind == _EqExp){
        eq_exp->GenerateIR();
        varName = make_unique<string>(*(eq_exp->varName));
    }else if(kind == _LAndExp_LAndOp_EqExp){
        land_exp->GenerateIR();
        eq_exp->GenerateIR();

        string leftVar = *(land_exp->varName);
        string rightVar = *(eq_exp->varName);
        
        if(land_op->compare("&&") == 0){
            unique_ptr<string> temp1 = make_unique<string>("\%" + to_string(temp_count++));
            unique_ptr<string> temp2 = make_unique<string>("\%" + to_string(temp_count++));
            cout << "  " << *temp1 << " = ne " << leftVar << ", " << "0" << "\n";
            cout << "  " << *temp2 << " = ne " << rightVar << ", " << "0" << "\n";
            varName = make_unique<string>("\%" + to_string(temp_count++));
            cout << "  " << *varName << " = and " << *temp1 << ", " << *temp2 << "\n";
        }else{
            assert(false);
        }
    }
}

void LOrExp::Dump() {
    cout << "LOrExp: { ";
    land_exp->Dump();
    cout << " }";
}

void LOrExp::GenerateIR(){
    if(kind == _LAndExp){
        land_exp->GenerateIR();
        varName = make_unique<string>(*(land_exp->varName));
    }else if(kind == _LOrExp_LOrOp_LAndExp){
        lor_exp->GenerateIR();
        land_exp->GenerateIR();

        string leftVar = *(lor_exp->varName);
        string rightVar = *(land_exp->varName);
        
        if(lor_op->compare("||") == 0){
            unique_ptr<string> temp1 = make_unique<string>("\%" + to_string(temp_count++));
            unique_ptr<string> temp2 = make_unique<string>("\%" + to_string(temp_count++));
            cout << "  " << *temp1 << " = ne " << leftVar << ", " << "0" << "\n";
            cout << "  " << *temp2 << " = ne " << rightVar << ", " << "0" << "\n";
            varName = make_unique<string>("\%" + to_string(temp_count++));
            cout << "  " << *varName << " = or " << *temp1 << ", " << *temp2 << "\n";
        }else{
            assert(false);
        }
    }
}

void ConstExp::Dump() {
    cout << "ConstExp: { ";
    exp->Dump();
    cout << " }";
}
void ConstExp::GenerateIR() {
    // Implementation for IR generation would go here
    exp->GenerateIR();
    varName = std::make_unique<string>(*(exp->varName));
}