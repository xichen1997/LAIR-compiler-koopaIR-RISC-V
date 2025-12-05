#include "../include/Koopa2RISCV.h"

using namespace std;
int offset = 0;
extern int total_variable_number;
unordered_map<koopa_raw_value_t, int> stack_offset_map;
unordered_set<koopa_raw_value_t> visited;

void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value) {
    // do nothing, termination node
   }
  
void Visit(const koopa_raw_return_t &ret) {
    if (ret.value->ty->tag == KOOPA_RTT_INT32) {
      Visit(ret.value);
      if(ret.value->kind.tag == KOOPA_RVT_INTEGER){
        cout << "  li a0, " << ret.value << endl;
        return;
      }
      int offset = stack_offset_map[ret.value];
      cout << "  lw a0, " << offset << "(sp)" << endl; 
    }else if(ret.value->ty->tag == KOOPA_RTT_UNIT){
      cout << "  ret";
    }
  }

void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value){
  // do nothing
  Visit(load.src);
}
void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value){
  Visit(store.value);
  Visit(store.dest);
  int offset = stack_offset_map[store.dest];
  // pure constant integer
  if(store.value->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  li t0, " << store.value->kind.data.integer.value << endl;
  }else{
    cout << "  lw t0, " << stack_offset_map[store.value] << "(sp)" << endl;
  }
  cout << "  sw t0, " << offset << "(sp)" << endl;
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
        Visit(kind.data.load, value);
        break;
      default:
        assert(false);
    }
  }
  
void Visit(const koopa_raw_basic_block_t &bb) { Visit(bb->insts); }
  
void Visit(const koopa_raw_function_t &func) {
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
    if(sp_gap > 2047){
      cout << "  li t0, " << sp_gap << endl;
      cout << "  add sp, sp, t0" << endl;
    }else{
      cout << "  addi sp, sp, " << sp_gap << endl;
    }
    cout << "  ret" << endl;
  }
  
void Visit(const koopa_raw_slice_t &slice) {
    for (size_t i = 0; i < slice.len; ++i) {
      auto ptr = slice.buffer[i];
      switch (slice.kind) {
        case KOOPA_RSIK_FUNCTION:
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
    string result = "t0"; // write to t0 and then write back to stack memory.

    if(binary.lhs->kind.tag == KOOPA_RVT_INTEGER){
      cout << "  li " << "t0, " <<  binary.lhs->kind.data.integer.value << endl;
    }else {
      cout << "  lw " << "t1, " << stack_offset_map[binary.lhs] <<"(sp)" << endl;
    }

    if(binary.rhs->kind.tag == KOOPA_RVT_INTEGER){
      cout << "  li " << "t0, " <<  binary.rhs->kind.data.integer.value << endl;
    }else {
      cout << "  lw " << "t1, " << stack_offset_map[binary.rhs] <<"(sp)" << endl;
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
  }
  