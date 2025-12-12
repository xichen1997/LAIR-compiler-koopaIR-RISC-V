#include "../include/AST.h"

unordered_map<string, string> func_type_map;
struct StackVariable{
    unordered_map<string, int> const_variable_table;
    unordered_map<string, string> initialized_variables;
    unordered_map<string, string> declared_variables;
} stackVariable;

vector<std::unique_ptr<StackVariable>> stack_variable_table;

int func_index = 0;
int temp_count= 0;
int total_variable_number = 0;
int max_parameter_number = 0;
bool is_function_called = 0;
std::unordered_map<std::string, int> func_total_vars_map;
std::unordered_map<std::string, int> func_max_params_map;
std::unordered_map<std::string, bool> func_is_called_map;

int if_control_index = 0; // to differentiate different if else block.
int logic_operator_index = 0; // use for && and || for shortcut
int local_variable_index = 0; // only increase while in a new block or function.
int while_index = 0; // for increase when a new while block is created.

stack<int> st_while; // for tracking the while_index


StackVariable* checkIsConstant(const string& tmp);
StackVariable* checkIsDeclared(const string& tmp);
StackVariable* checkIsInitialized(const string& tmp);

using namespace std;

void CompUnit::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "decl @getint(): i32" << endl;
    cout << "decl @getch(): i32" << endl;
    cout << "decl @getarray(*i32): i32" << endl;
    cout << "decl @putint(i32)" << endl;
    cout << "decl @putch(i32)" << endl;
    cout << "decl @putarray(i32, *i32)" << endl;
    cout << "decl @starttime()" << endl;
    cout << "decl @stoptime()" << endl;
    cout << endl;
    cout << endl;
    // establish func_type_map
    func_type_map["getint"]    = "int";
    func_type_map["getch"]     = "int";
    func_type_map["getarray"]  = "int";
    func_type_map["putint"]    = "void";
    func_type_map["putch"]     = "void";
    func_type_map["putarray"]  = "void";
    func_type_map["starttime"] = "void";
    func_type_map["stoptime"] = "void";

    // for(int i = 0; i < 8; ++i){
    //     total_variable_number_list.push_back(0);
    //     max_parameter_number_list.push_back(0);
    //     is_function_called_list.push_back(0);
    // }

    cout << endl;
    // add the function def manually.

    // for store the global variable definition.
    // keep the size = 1 at the beginning for global declaration.
    auto ptr = make_unique<StackVariable>();
    stack_variable_table.push_back(std::move(ptr));

    comp_unit_list->GenerateIR();
}

void CompUnitList::GenerateIR() {
    for(int i = 0; i < list.size(); ++i){
        list[i]->GenerateIR();
    }
}

void CompUnitItem::GenerateIR() {
    if(kind == _FuncDef) {
        // need to clean the status table/symbol for each function IR generation.
        while(!st_while.empty()){
            st_while.pop();
        }
        // stack_variable_table.resize(1);
        // clear count index
        while_index = 0;
        local_variable_index = 0;
        logic_operator_index = 0;
        if_control_index = 0;
        total_variable_number = 0;
        max_parameter_number = 0;
        is_function_called = false;
        temp_count = 0; 
        func_def->GenerateIR();
        // For generating RSIC-V code.
        // store the total_variable_number in the total_variable_number_list
        total_variable_number += max(max_parameter_number - 8, 0); // extra callee function parameters space.
        total_variable_number += temp_count;
        
        // for all the function in the translate unit
        string funcName = *(func_def->ident); // 获取函数名，如 "main"
        func_total_vars_map[funcName] = total_variable_number;
        func_max_params_map[funcName] = max_parameter_number;
        func_is_called_map[funcName] = is_function_called;
    }else{
        decl->GenerateIR();
    }
}

void FuncDef::GenerateIR() {
    // Implementation for IR generation would go here
    cout << "fun @" << *ident << "(";
    if(funcfparams != nullptr){
        cout << "@" << *(funcfparams->list[0]->ident) << ": i32";
        for(int i = 1; i < funcfparams->list.size(); ++i) {
            cout << ", @" << *(funcfparams->list[i]->ident) << ": i32";
        }
        total_variable_number += funcfparams->list.size(); // parameters temporary variables.
    }
    cout << ")";
    func_type->GenerateIR();
    cout << "{\n";
    cout << "\%entry_" << func_index++ <<":"<< endl;

    // define function type globally
    func_type_map[*ident] = *(func_type->functype);

    // define here, need a new stack variable table for parameters 
    auto ptr = std::make_unique<StackVariable>();
    stack_variable_table.push_back(std::move(ptr));

    if(funcfparams != nullptr){
        funcfparams->GenerateIR();
    }
    
    block->GenerateIR();
    if(!block->has_return) { // for the void type
        cout << "  ret" << endl;
    }
    cout << "}\n";

    // need to pop out the stack_variable_table
    stack_variable_table.pop_back();
}

void FuncRParams::GenerateIR() {
    // will generate the temorary var
    for(int i = 0; i < list.size(); ++i){
        list[i]->GenerateIR(); // exp->generateIR(); which will generate the tempvar
    }
} 

void FuncFParams::GenerateIR(){
    for(int i = 0; i < list.size(); ++i){
        list[i]->GenerateIR();
    }
}

void FuncFParam::GenerateIR(){
    total_variable_number++; // still need to add one because we apply for a new temporary vairable.
    StackVariable* current_variable_table_location = stack_variable_table.back().get();
    cout << "  %" << *ident << " = alloc i32" << endl;
    current_variable_table_location->declared_variables[*ident] = *ident;
    current_variable_table_location->initialized_variables[*ident] = *ident;
    cout << "  store @" << *ident << ", %" << *ident << endl;
    varName = std::make_unique<string>("\%" + *ident);
    // varName =  std::make_unique<string>("\%" + to_string(temp_count++));
    // cout << "  " << *varName  << " = load %" << *ident << endl;  

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
    // Need to seperate global declaration.
    if(stack_variable_table.size() == 1){
        StackVariable* current_variable_table_location = stack_variable_table.back().get();
        if(current_variable_table_location->declared_variables.count(*ident)){
            cerr << "Error: Redefinition of variable " << *ident << " at line " << lineno << endl;
            assert(false);
        }
        // use the name directly as the name of the global var
        current_variable_table_location->declared_variables[*ident] = *ident; 
        if(init_val){
            init_val->GenerateIR();
            string tmp = *(init_val->varName);
            cout << "global @" << *ident << " = alloc i32, " << tmp << endl; 
            current_variable_table_location->initialized_variables[*ident] = *ident;
        }else{
            cout << "global @" << *ident << " = alloc i32, zeroinit"  << endl; 
            current_variable_table_location->initialized_variables[*ident] = *ident;
        }
        return;
    }

    // Implementation for IR generation would go here
    StackVariable* current_variable_table_location = stack_variable_table.back().get();
    // cerr << "Generating IR for variable: " << *ident << endl;
    if(current_variable_table_location->declared_variables.count(*ident)){
        cerr << "Error: Redefinition of variable " << *ident << " at line " << lineno << endl;
        assert(false);
    }
    // declare it here
    current_variable_table_location->declared_variables[*ident] = *ident + "_" + to_string(local_variable_index);

    // local defined vairable need to increase total_variable_number by 1;
    total_variable_number++;

    cout << "  @" << *ident + "_" + to_string(local_variable_index) << " = alloc i32\n";

    if(init_val){
        init_val->GenerateIR();
        // check if the init_val variable is initialized and not temporary variables
        string tmp = *(init_val->varName);

        // comes from primaryexp so it must have value or it will exit earlier.
        cout << "  store " << *(init_val->varName) << ", @" << *ident + "_" + to_string(local_variable_index) << "\n";

        // initialize it later
        current_variable_table_location->initialized_variables[*ident] = *ident + "_" + to_string(local_variable_index);
    }
}

void InitVal::GenerateIR(){
    exp->GenerateIR();
    varName = std::make_unique<string>(*(exp->varName));
}

void FuncType::GenerateIR() {
    // Implementation for IR generation would go here
    if(functype->compare("int") == 0){
        cout << ": i32 ";
    } else if(functype->compare("void") == 0){
        // do nothing
        cout << " ";
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
    if(block_item_list->has_return) has_return = true;
    if(block_item_list->has_break) has_break = true;
    if(block_item_list->has_continue) has_continue = true;
    
    // free the variable space, unique_ptr will collect the space safely.
    stack_variable_table.pop_back();
}

void BlockItemList::GenerateIR() {
    for(auto& block_item : block_items){
        block_item->GenerateIR();
        if(block_item->has_return) {
            has_return = true;
            // early return here, if there is one block item has return then there is no need to generate other codes.
            return;
        }
        if(block_item->has_break) {
            has_break = true;
            return;
        }
        if(block_item->has_continue) {
            has_continue = true;
            return;
        }
    }
}

void BlockItem::GenerateIR() {
    if(kind == _Decl){
        decl->GenerateIR();
    } else if(kind == _Stmt){
        stmt->GenerateIR();
        if(stmt->has_return) has_return = true;
        if(stmt->has_break) has_break = true;
        if(stmt->has_continue) has_continue = true;
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
        cout << "  ret " << *(exp->varName) << endl;
        has_return = true;
    } else if(kind == _Exp){
        exp->GenerateIR();
    } else if(kind == _Empty){
        // do nothing
    } else if(kind == _Block){
        block->GenerateIR();
        if(block->has_return) has_return = true;
        if(block->has_break) has_break = true;
        if(block->has_continue) has_continue = true;
    } else if(kind == _If_Stmt){
        // add this line in the front in case of this is a nested if-else statement.   
        int tmp_index = if_control_index++;
        exp->GenerateIR();
        cout << "  br " << *(exp->varName) << ", \%then_" << tmp_index << ", \%else_" << tmp_index << endl;
        cout << endl;

        cout << "\%then_" << tmp_index << ":" << endl;
        ifstmt->GenerateIR();
        if(ifstmt->has_return){
            // do nothing, must have a ret 
        }else if(ifstmt->has_break || ifstmt->has_continue){ 
            // break or continue exist
        }
        else{
            cout << "  jump" << " \%end_" << tmp_index << endl;
        }
        cout << endl;

        cout << "\%else_" << tmp_index << ":" << endl;
        cout << "  jump" <<" \%end_" << tmp_index << endl;
        cout << endl;

        cout << "\%end_" << tmp_index << ":" << endl;

    } else if(kind == _If_Stmt_Else_Stmt){
        int tmp_index = if_control_index++;
        exp->GenerateIR();

        cout << "  br " << *(exp->varName) << ", \%then_" << tmp_index << ", \%else_" << tmp_index << endl;
        cout << endl;

        cout << "\%then_" << tmp_index << ":" << endl;
        ifstmt->GenerateIR();
        if(ifstmt->has_return){
            // do nothing, must have a ret 
        }else if(ifstmt->has_continue || ifstmt->has_break){
            // do nothing
        }
        else{
            cout << "  jump" << " \%end_" << tmp_index << endl;
        }
        cout << endl;

        cout << "\%else_" << tmp_index << ":" << endl;
        elsestmt->GenerateIR();
        if(elsestmt->has_return){
            // do nothing, must have a ret 
        }else if(elsestmt->has_break || elsestmt->has_continue){
            // do nothing
        }else{
            cout << "  jump" << " \%end_" << tmp_index << endl;
        }
        cout << endl;

        cout << "\%end_" << tmp_index << ":" << endl;
    } else if(kind == _While_Stmt){
        // the recursive situation might show up, use tep_while_index to prevent mess.
        while_index++;
        int tmp_while_index = while_index;
        st_while.push(tmp_while_index);

        auto while_entry_name = "\%while_entry_" + to_string(tmp_while_index);
        auto while_stmt_name = "\%while_stmt_" + to_string(tmp_while_index);
        auto while_next_name = "\%while_next_" + to_string(tmp_while_index);

        cout << "  jump " << while_entry_name << endl;
        cout <<endl; 

        // while entry block
        cout << while_entry_name << ":" << endl;
        exp->GenerateIR();
        cout << "  br " << *(exp->varName) << ", " << while_stmt_name << ", " << while_next_name << endl;
        cout << endl;
        
        // while stmt block
        cout <<  while_stmt_name << ":" << endl;
        whilestmt->GenerateIR();
        if(whilestmt->has_break || whilestmt->has_continue){
            // do nohting whehn the break and continue show up.
        }else if(whilestmt->has_return){
            // also donothing
        }else{
            cout << "  jump " << while_entry_name << endl;
            cout << endl;
        }

        // next block
        cout << while_next_name << ":" <<endl;
        st_while.pop();

    } else if (kind == _Break){
        if(st_while.empty()){
            cerr << "The break; statement is not inside a while loop" << endl;
            assert(false);
        }
        has_break = true;
        auto while_next_name = "\%while_next_" + to_string(st_while.top());

        cout << "  jump " << while_next_name << endl;
        cout << endl;

    } else if (kind == _Continue){
        if(st_while.empty()){
            cerr << "The continue; statement is not inside a while loop" << endl;
            assert(false);
        }
        has_continue = true;
        auto while_entry_name = "\%while_entry_" + to_string(st_while.top());

        cout << "  jump " << while_entry_name << endl;
        cout << endl;
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
        
        // first frame in stack means the parameters.
        StackVariable* first_variable_table_location = stack_variable_table[1].get();
        // *ident could be the variable(should be printed with @ in the beginning or the temporary vairable inside a function, start with % + alpha) 
        varName = std::make_unique<string>("%" + to_string(temp_count++));
        if(first_variable_table_location->declared_variables.count(*ident)){
            // %x ,etc,
            cout <<"  " << *varName << " = "<< "load %" << possible_variable_table_location->initialized_variables[*ident] <<endl;
        }else{
            cout <<"  " << *varName << " = "<< "load @" << possible_variable_table_location->initialized_variables[*ident] <<endl;
        }
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
    } else if(kind == _Func_No_Params){
        // call function  
        // need to calculate the max_parameters_number
        is_function_called = true;
        if(func_type_map[*func_name] == "int"){
            varName = std::make_unique<string>("\%" + to_string(temp_count++));
            cout << "  " << *varName << " = call @" << *func_name << "()" << endl;
        }else{
            varName = std::make_unique<string>("[PlaceHolderForVoidFunction]"); 
            cout << "  call @" << *func_name << "()" << endl;
        }
    } else if(kind == _Func_With_Params){
        // call function with params
        is_function_called = true;
        params->GenerateIR();
        max_parameter_number = max(max_parameter_number, static_cast<int>(params->list.size()));
        if(func_type_map[*func_name] != "void"){
            varName = std::make_unique<string>("\%" + to_string(temp_count++));
            cout << "  " << *varName << " = call @" << *func_name << "(";
        }else{
            varName = std::make_unique<string>("[PlaceHolderForVoidFunction]"); 
            cout << "  call @" << *func_name << "(";
        }

        cout << *(params->list[0]->varName);
        for(int i = 1; i < params->list.size(); ++i) {
            cout << ", ";
            cout <<  *(params->list[i]->varName);
        } 
        cout << ")" << endl;
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
        // increase the index 
        total_variable_number++;
        total_variable_number++;
        total_variable_number++;
        logic_operator_index++;

        if(land_op->compare("&&") == 0){
            // naming for variables
            auto endLabel = "\%logic_end_" + to_string(logic_operator_index);
            auto falseLabel = "\%logic_false_" + to_string(logic_operator_index);
            auto trueLabel = "\%logic_true_" + to_string(logic_operator_index);
            auto cond1 = "\%cond_l" + to_string(logic_operator_index);
            auto cond2 = "\%cond_r" + to_string(logic_operator_index);
            auto result = "result" + to_string(logic_operator_index);
            varName = make_unique<string>("\%" + to_string(temp_count++));

            // allocate the result:
            cout << "  @" << result << " = " << " alloc i32" << endl;
            cout << "  store 0, @" << result << endl;  

            land_exp->GenerateIR();
            string leftVar = *(land_exp->varName);

            cout << "  " << cond1 << " = ne " << leftVar <<", 0" << endl;
            cout << "  br " << cond1 << ", " << trueLabel << ", " << falseLabel << endl;
            cout << endl;
            
            // if the condition is true, need to calculate the second block.
            cout << trueLabel << ":"<<endl;
            eq_exp->GenerateIR();
            string rightVar = *(eq_exp->varName);
            cout << "  " << cond2 << " = ne " << rightVar << ", 0" << endl;
            cout << "  store " << cond2 << ", @" << result << endl; 
            cout << "  jump " << endLabel << endl;
            cout << endl;


            // if the condition is false, just give the result false;
            cout << falseLabel << ":" << endl;
            cout << "  store 0, @" << result << endl;
            cout << "  jump " << endLabel << endl;
            cout << endl;

            // end place
            cout << endLabel << ":" << endl;
            cout << "  " << *varName << " = " << "load @" << result << endl;   
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
        // increase index
        logic_operator_index++;
        total_variable_number++;
        total_variable_number++;
        total_variable_number++;
        if(lor_op->compare("||") == 0){
            // naming for variables
            auto endLabel = "\%logic_end_" + to_string(logic_operator_index);
            auto falseLabel = "\%logic_false_" + to_string(logic_operator_index);
            auto trueLabel = "\%logic_true_" + to_string(logic_operator_index);
            auto cond1 = "\%cond_l" + to_string(logic_operator_index);
            auto cond2 = "\%cond_r" + to_string(logic_operator_index);
            auto result = "result" + to_string(logic_operator_index);
            varName = make_unique<string>("\%" + to_string(temp_count++));

            // allocate the result:
            cout << "  @" << result << " = " << " alloc i32" << endl;
            cout << "  store 0, @" << result << endl;  

            lor_exp->GenerateIR();
            string leftVar = *(lor_exp->varName);

            cout << "  " << cond1 << " = ne " << leftVar <<", 0" << endl;
            cout << "  br " << cond1 << ", " << trueLabel << ", " << falseLabel << endl;
            cout << endl;

            // if the condition is false, just give the result false;
            cout << trueLabel << ":" << endl;
            cout << "  store 1, @" << result << endl;
            cout << "  jump " << endLabel << endl;
            cout << endl;
            
            // if the condition is true, need to calculate the second block.
            cout << falseLabel << ":"<<endl;
            land_exp->GenerateIR();
            string rightVar = *(land_exp->varName);
            cout << "  " << cond2 << " = ne " << rightVar << ", 0" << endl;
            cout << "  store " << cond2 << ", @" << result << endl; 
            cout << "  jump " << endLabel << endl;
            cout << endl;


            // end place
            cout << endLabel << ":" << endl;
            cout << "  " << *varName << " = " << "load @" << result << endl;   
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
