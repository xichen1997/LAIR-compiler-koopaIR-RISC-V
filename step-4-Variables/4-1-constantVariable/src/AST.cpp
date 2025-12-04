#include "../include/AST.h"

unordered_map<string, int> const_variable_table;
unordered_set<string> initialized_variables;
unordered_set<string> delacred_variables;

using namespace std;

int temp_count = 0;

void CompUnit::GenerateIR() {
    // Implementation for IR generation would go here
    func_def->GenerateIR();
}

void FuncDef::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "fun @" << *ident << "(): "; 
    func_type->GenerateIR();
    block->GenerateIR();
}

void Decl::GenerateIR(){
    // Implementation for IR generation would go here
    if(kind == _VarDecl){
        var_decl->GenerateIR();
    } else if(kind == _ConstDecl){
        const_decl->EvaluateConstValues();
    }
}

void ConstDecl::GenerateIR(){
    // do nothing
}

void ConstDecl::EvaluateConstValues(){
    const_def_list->EvaluateConstValues();
}

void ConstDefList::EvaluateConstValues(){
    for(auto& const_def : const_defs){
        const_def->EvaluateConstValues();
    }
}

void ConstDefList::GenerateIR(){
    // do nothing
} 

void ConstDef::EvaluateConstValues(){
    const_init_val->EvaluateConstValues();
    const_value = const_init_val->const_value;
    // update the const_variable_table
    if(const_variable_table.count(*ident) > 0){
        cerr << "Error: Redefinition of constant variable " << *ident << " at line " << lineno << endl;
        assert(false);
    }
    const_variable_table[*ident] = std::get<int>(const_value);
}

void ConstDef::GenerateIR(){
    // do nothing
}

void ConstInitVal::EvaluateConstValues(){
    const_exp->EvaluateConstValues();
    const_value = const_exp->const_value;
}

void ConstInitVal::GenerateIR(){
    // do nothing
}

void VarDecl::GenerateIR(){
    var_def_list->GenerateIR();
}

void VarDefList::GenerateIR(){
    for(auto& var_def : var_defs){
        var_def->GenerateIR();
    }
}

void VarDef::GenerateIR(){
    // Implementation for IR generation would go here
    if(delacred_variables.count(*ident) > 0){
        cerr << "Error: Redefinition of variable " << *ident << " at line " << lineno << endl;
        assert(false);
    }
    delacred_variables.insert(*ident);
    cout << "  @" << *ident << " = alloc i32\n";
    if(init_val){
        init_val->GenerateIR();
        // constant folding for init_val
        if(const_variable_table.count(*(init_val->varName)) > 0){
            cout << "  store " << const_variable_table[*(init_val->varName)] << ", @" << *ident << "\n";
            return;
        }  
        // check if the init_val variable is initialized and not temporary variables
        string tmp = *(init_val->varName);

        if(tmp[0]!='%' && !isdigit(tmp[0]) && initialized_variables.count(*(init_val->varName)) == 0){
            cerr << "Error: Use of uninitialized variable " << *(init_val->varName) << " at line " << lineno << endl;
            assert(false);
        }
        
        cout << "  store " << *(init_val->varName) << ", @" << *ident << "\n";
        initialized_variables.insert(*ident);
    }else{
        cerr << "uninitialized!" << endl;
    }
}

void InitVal::GenerateIR(){
    exp->GenerateIR();
    varName = std::make_unique<string>(*(exp->varName));
}

void FuncType::GenerateIR() {
    // Implementation for IR generation would go here
    if(functype->compare("int") == 0){
        cout << "i32 ";
    }
}

void Block::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "{\n";
    cout << "\%entry:\n";
    block_item_list->GenerateIR();
    cout << "\n}\n";
}

void BlockItemList::GenerateIR() {
    for(auto& block_item : block_items){
        block_item->GenerateIR();
    }
}

void BlockItem::GenerateIR() {
    if(kind == _Decl){
        decl->GenerateIR();
    } else if(kind == _Stmt){
        stmt->GenerateIR();
    }
}

void Stmt::GenerateIR() {
    if(kind == _Lval_Assign_Exp){
        exp->GenerateIR();
        // check if lval is not declared, if not then we add it to initialized_variables
        if(initialized_variables.count(*lval) == 0){
            initialized_variables.insert(*lval);
        }
        if(const_variable_table.count(*(exp->varName)) > 0){
            cout << "  store " << const_variable_table[*(exp->varName)] << ", @" << *lval << "\n";
            return;
        }  
        cout << "  store " << *(exp->varName) << ", @" << *lval << "\n";
    } else if(kind == _Return_Exp){
        exp->GenerateIR();
        string tmp = *(exp->varName);
        if(const_variable_table.count(*(exp->varName)) > 0){
            cout << "  ret " << const_variable_table[*(exp->varName)] << "\n";
            return;
        } 
        
        if(tmp[0]!='%' && !isdigit(tmp[0]) && initialized_variables.count(*(exp->varName)) == 0){
            cerr << "Use uninitialized variable as return value" << endl;
            assert(false);
        }
        if(tmp[0]!='%' && !isdigit(tmp[0])){
            cout << "  ret @" << *(exp->varName) << "\n";
            return;
        }
        cout << "  ret " << *(exp->varName) << "\n";
    }
}

void Number::GenerateIR() {
    // Implementation for IR generation would go here
    // cout << to_string(int_const);
}

void Exp::GenerateIR() {
    // Implementation for IR generation would go here
    lor_exp->GenerateIR();
    varName = std::make_unique<string>(*(lor_exp->varName));
}

void Exp::EvaluateConstValues(){
    lor_exp->EvaluateConstValues();
    const_value = lor_exp->const_value;
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

void PrimaryExp::EvaluateConstValues(){
    if(kind == _Exp){
        exp->EvaluateConstValues();
        const_value = exp->const_value;
    } else if(kind == _Number){
        const_value = number->int_const;
    } else if(kind == _Lval){
        // lookup the const_variable_table to get the value
        if(const_variable_table.count(*ident) == 0){
            cerr << "Error: Undefined constant variable " << *ident << " at line " << lineno << endl;
            assert(false);
        }
        const_value = const_variable_table[*ident];
    } else{
        // give error information
        cerr << "Error: Invalid PrimaryExp kind for EvaluateConstValues" << endl;
        assert(false);
    }
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
            if(const_variable_table.count(*(unary_exp->varName)) > 0){
                cout << "add 0, " << const_variable_table[*(unary_exp->varName)] << "\n";
            }else{
                cout << "add 0, " << *(unary_exp->varName) << "\n";
            }
        }else if(unary_op->compare("-") == 0){
            if(const_variable_table.count(*(unary_exp->varName)) > 0){
                cout << "sub 0, " << const_variable_table[*(unary_exp->varName)] << "\n";
            }else{
                cout << "sub 0, " << *(unary_exp->varName) << "\n";
            }
        } else if(unary_op->compare("!") == 0){
            if(const_variable_table.count(*(unary_exp->varName)) > 0){
                cout << "eq 0, " << const_variable_table[*(unary_exp->varName)] << "\n";
            }else{
                cout << "eq 0, " << *(unary_exp->varName) << "\n";
            }
        }
    }
}

void UnaryExp::EvaluateConstValues(){
    if(kind == _PrimaryExp){
        primary_exp->EvaluateConstValues();
        const_value = primary_exp->const_value;
    } else if(kind == _UnaryOp_UnaryExp){
        unary_exp->EvaluateConstValues();
        int val = std::get<int>(unary_exp->const_value);
        if(unary_op->compare("+") == 0){
            const_value = val;
        } else if(unary_op->compare("-") == 0){
            const_value = -val;
        } else if(unary_op->compare("!") == 0){
            const_value = (val == 0) ? 1 : 0;
        }
    } else{
        // give error information
        cerr << "Error: Invalid UnaryExp kind for EvaluateConstValues" << endl;
        assert(false);
    }
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

        if(const_variable_table.count(leftVar) > 0){
            leftVar = to_string(const_variable_table[leftVar]);
        } 
        if(const_variable_table.count(rightVar) > 0){
            rightVar = to_string(const_variable_table[rightVar]);
        }

        if(mul_op->compare("*") == 0){
            cout << "  " << *varName << " = mul " << leftVar << ", " << rightVar << "\n";
        } else if(mul_op->compare("/") == 0){
            cout << "  " << *varName << " = div " << leftVar << ", " << rightVar << "\n";
        } else if(mul_op->compare("%") == 0){
            cout << "  " << *varName << " = mod " << leftVar << ", " << rightVar << "\n";
        } else{
            cerr << "Error: Invalid MulExp operator for GenerateIR" << endl;
            assert(false);
        }
    }
}

void MulExp::EvaluateConstValues(){
    if(kind == _UnaryExp){
        unary_exp->EvaluateConstValues();
        const_value = unary_exp->const_value;
    }
    else if(kind == _MulExp_MulOp_UnaryExp){
        mul_exp->EvaluateConstValues();
        unary_exp->EvaluateConstValues();

        int leftVal = std::get<int>(mul_exp->const_value);
        int rightVal = std::get<int>(unary_exp->const_value);
        
        if(mul_op->compare("*") == 0){
            const_value = leftVal * rightVal;
        } else if(mul_op->compare("/") == 0){
            const_value = leftVal / rightVal;
        } else if(mul_op->compare("%") == 0){
            const_value = leftVal % rightVal;
        } else{
            cerr << "Error: Invalid MulExp operator for EvaluateConstValues" << endl;
            assert(false);
        }
    }
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

        if(const_variable_table.count(leftVar) > 0){
            leftVar = to_string(const_variable_table[leftVar]);
        }
        if(const_variable_table.count(rightVar) > 0){
            rightVar = to_string(const_variable_table[rightVar]);
        }
        
        if(add_op->compare("+") == 0){
            cout << "  " << *varName << " = add " << leftVar << ", " << rightVar << "\n";
        } else if(add_op->compare("-") == 0){
            cout << "  " << *varName << " = sub " << leftVar << ", " << rightVar << "\n";
        } else{
            assert(false);
        }
    }
}

void AddExp::EvaluateConstValues(){
    if(kind == _MulExp){
        mul_exp->EvaluateConstValues();
        const_value = mul_exp->const_value;
    }else if(kind == _AddExp_AddOp_MulExp){
        add_exp->EvaluateConstValues();
        mul_exp->EvaluateConstValues();

        int leftVal = std::get<int>(add_exp->const_value);
        int rightVal = std::get<int>(mul_exp->const_value);
        
        if(add_op->compare("+") == 0){
            const_value = leftVal + rightVal;
        } else if(add_op->compare("-") == 0){
            const_value = leftVal - rightVal;
        } else{
            cerr << "Error: Invalid AddExp operator for EvaluateConstValues" << endl;
            assert(false);
        }
    }
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

        if(const_variable_table.count(leftVar) > 0){
            leftVar = to_string(const_variable_table[leftVar]);
        }
        if(const_variable_table.count(rightVar) > 0){
            rightVar = to_string(const_variable_table[rightVar]);
        }
        
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

void RelExp::EvaluateConstValues(){
    if(kind == _AddExp){
        add_exp->EvaluateConstValues();
        const_value = add_exp->const_value;
    }else if(kind == _RelExp_RelOp_AddExp){
        rel_exp->EvaluateConstValues();
        add_exp->EvaluateConstValues();

        int leftVal = std::get<int>(rel_exp->const_value);
        int rightVal = std::get<int>(add_exp->const_value);
        
        if(rel_op->compare("<") == 0){
            const_value = (leftVal < rightVal) ? 1 : 0;
        } else if(rel_op->compare(">") == 0){
            const_value = (leftVal > rightVal) ? 1 : 0;
        } else if(rel_op->compare("<=") == 0){
            const_value = (leftVal <= rightVal) ? 1 : 0;
        } else if(rel_op->compare(">=") == 0){
            const_value = (leftVal >= rightVal) ? 1 : 0;
        } else{
            cerr << "Error: Invalid RelExp operator for EvaluateConstValues" << endl;
            assert(false);
        }
    }
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

        if(const_variable_table.count(leftVar) > 0){
            leftVar = to_string(const_variable_table[leftVar]);
        }
        if(const_variable_table.count(rightVar) > 0){
            rightVar = to_string(const_variable_table[rightVar]);
        }
        
        if(eq_op->compare("==") == 0){
            cout << "  " << *varName << " = eq " << leftVar << ", " << rightVar << "\n";
        } else if(eq_op->compare("!=") == 0){
            cout << "  " << *varName << " = ne " << leftVar << ", " << rightVar << "\n";
        } else{
            assert(false);
        }
    }
}

void EqExp::EvaluateConstValues(){
    if(kind == _RelExp){
        rel_exp->EvaluateConstValues();
        const_value = rel_exp->const_value;
    }else if(kind == _EqExp_EqOp_RelExp){
        eq_exp->EvaluateConstValues();
        rel_exp->EvaluateConstValues();

        int leftVal = std::get<int>(eq_exp->const_value);
        int rightVal = std::get<int>(rel_exp->const_value);
        
        if(eq_op->compare("==") == 0){
            const_value = (leftVal == rightVal) ? 1 : 0;
        } else if(eq_op->compare("!=") == 0){
            const_value = (leftVal != rightVal) ? 1 : 0;
        } else{
            cerr << "Error: Invalid EqExp operator for EvaluateConstValues" << endl;
            assert(false);
        }
    }
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

        if(const_variable_table.count(leftVar) > 0){
            leftVar = to_string(const_variable_table[leftVar]);
        }
        if(const_variable_table.count(rightVar) > 0){
            rightVar = to_string(const_variable_table[rightVar]);
        }
        
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

void LAndExp::EvaluateConstValues(){
    if(kind == _EqExp){
        eq_exp->EvaluateConstValues();
        const_value = eq_exp->const_value;
    }else if(kind == _LAndExp_LAndOp_EqExp){
        land_exp->EvaluateConstValues();
        eq_exp->EvaluateConstValues();

        int leftVal = std::get<int>(land_exp->const_value);
        int rightVal = std::get<int>(eq_exp->const_value);
        
        if(land_op->compare("&&") == 0){
            const_value = (leftVal != 0 && rightVal != 0) ? 1 : 0;
        }else{
            cerr << "Error: Invalid LAndExp operator for EvaluateConstValues" << endl;
            assert(false);
        }
    }
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

        if(const_variable_table.count(leftVar) > 0){
            leftVar = to_string(const_variable_table[leftVar]);
        }
        if(const_variable_table.count(rightVar) > 0){
            rightVar = to_string(const_variable_table[rightVar]);
        }
        
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

void LOrExp::EvaluateConstValues(){
    if(kind == _LAndExp){
        land_exp->EvaluateConstValues();
        const_value = land_exp->const_value;
    }else if(kind == _LOrExp_LOrOp_LAndExp){
        lor_exp->EvaluateConstValues();
        land_exp->EvaluateConstValues();

        int leftVal = std::get<int>(lor_exp->const_value);
        int rightVal = std::get<int>(land_exp->const_value);
        
        if(lor_op->compare("||") == 0){
            const_value = (leftVal != 0 || rightVal != 0) ? 1 : 0;
        }else{
            cerr << "Error: Invalid LOrExp operator for EvaluateConstValues" << endl;
            assert(false);
        }
    }
}


void ConstExp::GenerateIR() {
    // do nothing
}

void ConstExp::EvaluateConstValues(){
    exp->EvaluateConstValues();
    const_value = exp->const_value;
}

