#include "../include/Koopa2RISCV.h"

using namespace std;
int offset = 0;
extern vector<int> total_variable_number_list;
extern int total_variable_number;
unordered_map<koopa_raw_value_t, int> stack_offset_map;
unordered_set<koopa_raw_value_t> visited;
unordered_map<koopa_raw_value_t, int> integer_map;

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
    integer_map[value] = integer.value;
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

  // return to the previous stack frame
  if(sp_gap > 2047){
    cout << "  li t0, 0" << sp_gap << endl;
    cout << "  add sp, sp, t0" << endl;
  }else{
    cout << "  addi sp, sp, " << sp_gap << endl;
  }
  cout << "  ret" << endl;
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
  // Visit(call.callee);
  cout << "  call " << *(call.callee->name) << endl;
  // Visit(call.args);
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
      case KOOPA_RVT_UNDEF:
        stack_offset_map[value] = offset*4;
        offset++;
        break;
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
      default:
        assert(false);
    }
  }
  
void Visit(const koopa_raw_basic_block_t &bb) { 
  // cout << printBBName(bb->name) << ":" << endl;
  Visit(bb->insts); 
}
  
void Visit(const koopa_raw_function_t &func) {
    // clean status, only keep total_varaible_number
    total_variable_number = total_variable_number_list.front();
    total_variable_number_list.erase(total_variable_number_list.begin());
    stack_offset_map.clear();
    visited.clear();
    integer_map.clear();

    cout << "  .globl " << func->name + 1 << endl;  // skip the '@' character
    cout << func->name + 1 << ":" << endl;
    int sp_gap = total_variable_number;
    sp_gap = sp_gap * 4;
    sp_gap = (sp_gap + 15) / 16 * 16;
    
    if(sp_gap > 2048){
      cout << "  li t0, " << -sp_gap << endl;
      cout << "  add sp, sp, t0" << endl;
    }else{
      cout << "  addi sp, sp, " << -sp_gap << endl;
    }
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
  