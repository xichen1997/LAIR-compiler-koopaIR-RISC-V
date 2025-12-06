#include "../include/AST.h"

struct StackVariable{
    unordered_map<string, int> const_variable_table;
    unordered_map<string, string> initialized_variables;
    unordered_map<string, string> declared_variables;
} stackVariable;

vector<std::unique_ptr<StackVariable>> stack_variable_table;

int temp_count = 0;
int total_variable_number = 0;

int local_variable_index = 0;


StackVariable* checkIsConstant(const string& tmp);
StackVariable* checkIsDeclared(const string& tmp);
StackVariable* checkIsInitialized(const string& tmp);

using namespace std;

void CompUnit::GenerateIR() {
    // Implementation for IR generation would go here
    func_def->GenerateIR();
    total_variable_number += temp_count; // temp vars and vars total.
}

void FuncDef::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "fun @" << *ident << "(): "; 
    func_type->GenerateIR();
    cout << "{\n";
    cout << "\%entry:\n";
    block->GenerateIR();
    cout << "\n}\n";
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
    StackVariable* current_variable_table_location = stack_variable_table.back().get();
    if(current_variable_table_location->const_variable_table.count(*ident) > 0){
        cerr << "Error: Redefinition of constant variable " << *ident << " at line " << lineno << endl;
        assert(false);
    }
    current_variable_table_location->const_variable_table[*ident] = std::get<int>(const_value);
    
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
    StackVariable* current_variable_table_location = stack_variable_table.back().get();
    cerr << "Generating IR for variable: " << *ident << endl;
    if(current_variable_table_location->declared_variables.count(*ident)){
        cerr << "Error: Redefinition of variable " << *ident << " at line " << lineno << endl;
        assert(false);
    }
    // declare it here
    current_variable_table_location->declared_variables[*ident] = *ident + "___" + to_string(local_variable_index);

    total_variable_number++;

    cout << "  @" << *ident + "___" + to_string(local_variable_index) << " = alloc i32\n";

    if(init_val){
        init_val->GenerateIR();
        // check if the init_val variable is initialized and not temporary variables
        string tmp = *(init_val->varName);

        StackVariable* possible_variable_table_location = checkIsInitialized(*ident);
        // comes from primaryexp so it must have value or it will exit earlier.
        cout << "  store " << *(init_val->varName) << ", @" << *ident + "___" + to_string(local_variable_index) << "\n";

        // initialize it later
        current_variable_table_location->initialized_variables[*ident] = *ident + "___" + to_string(local_variable_index);
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
    // create new local variables space
    auto ptr = std::make_unique<StackVariable>();
    stack_variable_table.push_back(std::move(ptr));
    
    // monotonic increasing to combine with variable name 
    local_variable_index++; 

    // Implementation for IR generation would go here
    block_item_list->GenerateIR();
    
    // free the variable space, unique_ptr will collect the space safely.
    stack_variable_table.pop_back();
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
        StackVariable* possible_variable_table_location = checkIsDeclared(*lval);
        if(possible_variable_table_location == nullptr){
            cerr << "Use Undeclared Vairables to assign Value" << endl;
            assert(false);
        }
        // const value(or Number) or temparory value
        cout << "  store " << *(exp->varName) << ", @" << possible_variable_table_location->declared_variables[*lval] << "\n";
        // then it must be initialized variables, can be used in the future.
        possible_variable_table_location->initialized_variables[*lval] = possible_variable_table_location->declared_variables[*lval];
    } else if(kind == _Return_Exp){
        exp->GenerateIR();
        cout << "  ret " << *(exp->varName);
    } else if(kind == _Exp){
        exp->GenerateIR();
    } else if(kind == _Empty){
        // do nothing
    } else if(kind == _Block){
        block->GenerateIR();
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
    varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        // already do the constant folding. So the upstream node don't need to care.
        StackVariable* possible_variable_table_location = checkIsConstant(*ident);
        if(possible_variable_table_location != nullptr){
            varName = std::make_unique<string>(to_string(possible_variable_table_location->const_variable_table[*ident]));
            return;
        }

        // check if the vairable is not declared 
        possible_variable_table_location = checkIsDeclared(*ident);
        if(possible_variable_table_location == nullptr){
            cerr << "Error: variable '" << *ident << "' is not declared" << endl;
            assert(false);
        }

        // is initialized?
        possible_variable_table_location = checkIsInitialized(*ident); 
        if(possible_variable_table_location == nullptr){
            cerr << "Error: variable '" << *ident << "' is not initiailized" << endl;
            assert(false);
        }

        // *ident is the original value, when generating IR must use the hashed value.
        varName = std::make_unique<string>("%" + to_string(temp_count++));
        cout <<"  " << *varName << " = "<< "load @" << possible_variable_table_location->initialized_variables[*ident] <<endl;
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
        StackVariable* possible_variable_table_location = checkIsConstant(*ident);
        if(possible_variable_table_location == nullptr){
            cerr << "Error: use not constant symbol to generate another constant variable." << endl;
            assert(false);
        }
        const_value = possible_variable_table_location->const_variable_table[*ident];
    } else{
        // give error information
        cerr << "Error: Invalid PrimaryExp kind for EvaluateConstValues" << endl;
        assert(false);
    }
    varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        
        if(unary_op->compare("-") == 0){
            cout << "sub 0, " << *(unary_exp->varName) << endl;
        } else if(unary_op->compare("!") == 0){
            cout << "eq 0, " << *(unary_exp->varName) << endl;
        } else if(unary_op->compare("+") == 0){
            cout << "add 0, " << *(unary_exp->varName) << endl;
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
    varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
    varName = make_unique<string>(to_string(std::get<int>(const_value)));
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

void AddExp::EvaluateConstValues(){
    if(kind == _MulExp){
        mul_exp->EvaluateConstValues();
        const_value = mul_exp->const_value;
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
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
        varName = make_unique<string>(to_string(std::get<int>(const_value)));
    }
}


void ConstExp::GenerateIR() {
    // do nothing
}

void ConstExp::EvaluateConstValues(){
    exp->EvaluateConstValues();
    const_value = exp->const_value;
}



StackVariable* checkIsDeclared(const string &symbol){
    for(int i = stack_variable_table.size() - 1; i >=0; i--){
        StackVariable* ptr = stack_variable_table[i].get();
        if(ptr->declared_variables.count(symbol)){
            return ptr;
        }
    }
    return nullptr;
}

StackVariable* checkIsInitialized(const string &symbol){
    for(int i = stack_variable_table.size() - 1; i >=0; i--){
        StackVariable* ptr = stack_variable_table[i].get();
        if(ptr->initialized_variables.count(symbol)){
            return ptr;
        }
    }
    return nullptr;
}

StackVariable* checkIsConstant(const string &symbol){
    for(int i = stack_variable_table.size() - 1; i >=0; i--){
        StackVariable* ptr = stack_variable_table[i].get();
        if(ptr->const_variable_table.count(symbol)){
            return ptr;
        }
    }
    return nullptr;
}
