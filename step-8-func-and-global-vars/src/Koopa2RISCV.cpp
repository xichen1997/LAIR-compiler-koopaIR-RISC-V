#include "../include/Koopa2RISCV.h"

using namespace std;
int offset = 0; // index for allocating 
extern vector<int> total_variable_number_list;
extern vector<int> max_parameter_number_list;
extern vector<bool> is_function_called_list;
extern int max_parameter_number;
extern int total_variable_number;
extern bool is_function_called;
unordered_map<koopa_raw_value_t, int> stack_offset_map; // string -> offset | index for these temporary variable
unordered_set<koopa_raw_value_t> visited;
// unordered_map<koopa_raw_value_t, int> integer_map;

string printBBName(const string & str){
  return str.substr(1);
}

void LoadFromStack(const std::string &reg, int offset) {
  if (offset > 2047 || offset < -2048) {
      cout << "  li t1, " << offset << endl;
      cout << "  add t1, sp, t1" << endl;
      cout << "  lw " << reg << ", 0(t1)" << endl;
  } else {
      cout << "  lw " << reg << ", " << offset << "(sp)" << endl;
  }
}

void StoreToStack(const std::string &reg, int offset) {
  // in register
  if (offset > 2047 || offset < -2048) {
      cout << "  li t1, " << offset << endl;
      cout << "  add t1, sp, t1" << endl;
      cout << "  sw " << reg << ", 0(t1)" << endl;
  } else {
      cout << "  sw " << reg << ", " << offset << "(sp)" << endl;
  }
}

void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value) {
    // do nothing, termination node
    // integer_map[value] = integer.value;
   }
  
void Visit(const koopa_raw_return_t &ret) {

  int sp_gap = total_variable_number;
  sp_gap = sp_gap * 4;
  sp_gap = (sp_gap + 15) / 16 * 16;

  // if the ret.value exists, then check the value is a variable
  if (ret.value) {
    Visit(ret.value);
    if(ret.value->kind.tag == KOOPA_RVT_INTEGER){
      cout << "  li a0, " << ret.value->kind.data.integer.value << endl;
    }else{
      LoadFromStack("a0", stack_offset_map[ret.value]);
    }
  }

  if(is_function_called){
    cout << "  lw ra, " << sp_gap - 4 << "(sp)" << endl;
  }

  // return to the previous stack frame
  if(sp_gap > 2047){
    cout << "  li t0, 0" << sp_gap << endl;
    cout << "  add sp, sp, t0" << endl;
  }else if(sp_gap <= 2047 && sp_gap > 0) {
    cout << "  addi sp, sp, " << sp_gap << endl;
  }
  cout << "  ret" << endl;
  cout << endl;
}

void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value){
  Visit(branch.cond);
  if(branch.cond->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  li t0, " << branch.cond->kind.data.integer.value << endl;
  }else{
    LoadFromStack("t0", stack_offset_map[branch.cond]);
  }
  cout << "  bnez " << "t0" << ", " << printBBName(branch.true_bb->name) << endl;
  cout << "  j " << printBBName(branch.false_bb->name) << endl;
  cout << endl;
}

void Visit(const koopa_raw_jump_t &jump, const koopa_raw_value_t &value){
  cout << "  j " << printBBName(jump.target->name) <<endl; 
  // Visit(jump.target);

}

void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value){
  // do nothing
  Visit(load.src);
  stack_offset_map[value] = offset*4;
  offset++;
  LoadFromStack("t0", stack_offset_map[load.src]);
  StoreToStack("t0", stack_offset_map[value]);
}

void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value){
  Visit(store.value);
  Visit(store.dest);
  // pure constant integer
  // cerr << store.value->kind.data.integer.value << endl;
  // cerr << offset << endl;

  // for loading parameters from registers
  if(stack_offset_map[store.value] < 0) { // load from register
    StoreToStack("a" + to_string(- stack_offset_map[store.value] - 1), stack_offset_map[store.dest]);
    return;
  }

  // for loading extra parameters from last stack frame,
  // need to add current stack frame size and the offset.
  // if they have same name like %x = @x, which is loading the parameters, we should compare and add
  // a gap, which is the current total_variable_number.
  // 
  // check exist before use. sometimes the store value is a integer without name.
  if(store.value->name && store.dest->name){
    string a((store.value->name));
    string b((store.dest->name));
    if(a.substr(1) == b.substr(1)){
      stack_offset_map[store.value] = total_variable_number + stack_offset_map[store.value];
    }
  }

  // normal store command
  if(store.value->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  li t0, " << store.value->kind.data.integer.value << endl;
  }else{
    LoadFromStack("t0", stack_offset_map[store.value]);
  }
  StoreToStack("t0", stack_offset_map[store.dest]);
}

// void Visit(const koopa_raw_function_t & callee){
//   cout << "  " << *(callee->name) << endl;
// }

void Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value){
  // put the constant or variable into proper position
  for(int i = 0; i < call.args.len; ++i){
    koopa_raw_value_t arg = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);

    if(i < 8){
      if (arg->kind.tag == KOOPA_RVT_INTEGER){
        cout << "  li a"<< i <<", " << arg->kind.data.integer.value << endl;
      }else{
        LoadFromStack("a" + to_string(i), stack_offset_map[arg]);
      }
    }else{
      int location = (i - 8) * 4;
      if (arg->kind.tag == KOOPA_RVT_INTEGER){
        cout << "  li t0, " << arg->kind.data.integer.value << endl;
      }else{
        LoadFromStack("t0", stack_offset_map[arg]);
      }
      StoreToStack("t0", location);
    }
  }

  string func_name = string(call.callee->name).substr(1);
  cout << "  call " << func_name << endl;

  // if function is void type, then there is no return value
  if(call.callee->ty->tag == KOOPA_RTT_INT32){
    stack_offset_map[value] = offset*4;
    offset++;
    StoreToStack("a0", stack_offset_map[value]);
  }

}

void Visit(const koopa_raw_func_arg_ref_t &func_arg_ref, const koopa_raw_value_t &value){
  // do nothing here
}
  
void Visit(const koopa_raw_value_t &value) {
  
    if (visited.count(value)) return;
    visited.insert(value);
  
    const auto &kind = value->kind;
    switch (kind.tag) {
      case KOOPA_RVT_RETURN:
        Visit(kind.data.ret);
        break;
      case KOOPA_RVT_INTEGER:
        Visit(kind.data.integer, value);
        break;
      case KOOPA_RVT_BINARY:
        Visit(kind.data.binary, value);
        break;
      case KOOPA_RVT_ALLOC:
        // no visit function since no value need to be saved.
        stack_offset_map[value] = offset*4; // value -> stack sp offset (of course need to * 4)
        offset++;
        break;
      // case KOOPA_RVT_UNDEF:
      //   stack_offset_map[value] = offset*4;
      //   offset++;
      //   break;
      case KOOPA_RVT_LOAD:
        Visit(kind.data.load, value);
        break;
      case KOOPA_RVT_STORE:
        Visit(kind.data.store, value);
        break;
      case KOOPA_RVT_BRANCH:
        Visit(kind.data.branch, value);
        break;
      case KOOPA_RVT_JUMP:
        Visit(kind.data.jump, value);
        break;
      case KOOPA_RVT_CALL:
        Visit(kind.data.call, value);
        break;
      case KOOPA_RVT_FUNC_ARG_REF:
        Visit(kind.data.func_arg_ref, value);
        break;
      default:
        cerr << "Unhandled value tag = " << kind.tag << endl;
        assert(false);
    }
  }
  
void Visit(const koopa_raw_basic_block_t &bb) { 
  cout << printBBName(bb->name) << ":" << endl;
  Visit(bb->insts); 
}
  
void Visit(const koopa_raw_function_t &func) {
    // get the status total_variable_number, 
    //                max_parameter_number,
    //                is_function_called.
    total_variable_number = total_variable_number_list.front();
    max_parameter_number = max_parameter_number_list.front();
    is_function_called = is_function_called_list.front();
    total_variable_number_list.erase(total_variable_number_list.begin());
    max_parameter_number_list.erase(max_parameter_number_list.begin());
    is_function_called_list.erase(is_function_called_list.begin());

    // offset means the start position to arrange the temporary variables
    offset = max(max_parameter_number - 8,0);
    stack_offset_map.clear();
    visited.clear();
    //integer_map.clear();


    cout << "  .globl " << func->name + 1 << endl;  // skip the '@' character
    cout << func->name + 1 << ":" << endl;
    int sp_gap = total_variable_number;
    sp_gap = sp_gap * 4;
    sp_gap = (sp_gap + 15) / 16 * 16;


    // need to load new stack_offset_map for the parameters.
    // params
    for(int i = 0; i < func->params.len; ++i){
      // need to prepare parameters
      Visit(reinterpret_cast<koopa_raw_value_t>(func->params.buffer[i]));
      if(i < 8) {
        stack_offset_map[reinterpret_cast<koopa_raw_value_t>(func->params.buffer[i])] = -i - 1;
      }else{
        stack_offset_map[reinterpret_cast<koopa_raw_value_t>(func->params.buffer[i])]  = (i - 8)*4 + sp_gap;
      }
    }
    
    if(sp_gap > 2048){
      cout << "  li t0, " << -sp_gap << endl;
      cout << "  add sp, sp, t0" << endl;
    }else if(sp_gap <= 2048 && sp_gap > 0){
      cout << "  addi sp, sp, " << -sp_gap << endl;
    }

    // check if the func will call other func, if yes then consider store ra value.
    if(is_function_called){
      cout << "  sw ra, " << sp_gap - 4 << "(sp)" << endl;
    }

    // This will do the koopaIR stuff. The parameters initialization is also included
    Visit(func->bbs);

  }
  
void Visit(const koopa_raw_slice_t &slice) {
    for (size_t i = 0; i < slice.len; ++i) {
      auto ptr = slice.buffer[i];
      switch (slice.kind) {
        case KOOPA_RSIK_FUNCTION:
          // clear all the status // total_variable_number should also update.
          Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
          break;
        case KOOPA_RSIK_BASIC_BLOCK:
          Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
          break;
        case KOOPA_RSIK_VALUE:
          Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
          break;
        default:
          assert(false);
      }
    }
  }
  
void Visit(const koopa_raw_program_t &program) {
    cout << "  .text" << endl;
    Visit(program.values);
    Visit(program.funcs);
  }
  
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value) {
  
    Visit(binary.lhs);
    Visit(binary.rhs);

    string lhs = "t0";
    string rhs = "t1";
    stack_offset_map[value] = offset*4;
    offset++;

    if(binary.lhs->kind.tag == KOOPA_RVT_INTEGER){
      cout << "  li " << "t0, " <<  binary.lhs->kind.data.integer.value << endl;
    }else {
      LoadFromStack("t0", stack_offset_map[binary.lhs]);
    }

    if(binary.rhs->kind.tag == KOOPA_RVT_INTEGER){
      cout << "  li " << "t1, " <<  binary.rhs->kind.data.integer.value << endl;
    }else {
      LoadFromStack("t1", stack_offset_map[binary.rhs]);
    }

    switch (binary.op){
      case KOOPA_RBO_SUB:
        cout << "  sub " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_EQ:
        cout << "  xor " << "t0" << ", " <<  lhs << ", " << rhs << endl;
        cout << "  seqz " << "t0" << ", " << "t0" << endl;
      break;
      case KOOPA_RBO_ADD:
        cout << "  add " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_MUL:
        cout << "  mul " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_DIV:
        cout << "  div " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_MOD:
        cout << "  rem " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_NOT_EQ:
        cout << "  xor " << "t0" << ", " <<  lhs << ", " << rhs << endl;
        cout << "  snez " << "t0" << ", " << "t0" << endl;
      break;
      case KOOPA_RBO_LT:
        cout << "  slt " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_GT:
        cout << "  slt " << "t0" << ", " << rhs << ", " << lhs << endl;
      break;
      case KOOPA_RBO_LE:
        cout << "  slt " << "t0" << ", " << rhs << ", " << lhs << endl;
        cout << "  seqz " << "t0" << ", " << "t0" << endl;
      break;
      case KOOPA_RBO_GE:
        cout << "  slt " << "t0" << ", " << lhs << ", " << rhs << endl;
        cout << "  seqz " << "t0" << ", " << "t0" << endl;
      break;
      case KOOPA_RBO_AND:
        cout << "  and " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      case KOOPA_RBO_OR:
        cout << "  or " << "t0" << ", " << lhs << ", " << rhs << endl;
      break;
      default:
        assert(false);
    }
    StoreToStack("t0", stack_offset_map[value]);
  }
  