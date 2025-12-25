#include "../include/Koopa2RISCV.h"

using namespace std;
int offset = 0; // index for allocating 
extern std::unordered_map<std::string, int> func_total_vars_map;
extern std::unordered_map<std::string, int> func_max_params_map;
extern std::unordered_map<std::string, bool> func_is_called_map;
extern int max_parameter_number;
extern int total_variable_number;
extern bool is_function_called;
unordered_map<koopa_raw_value_t, int> stack_offset_map; // string -> offset | index for these temporary variable
unordered_set<koopa_raw_value_t> visited;
// unordered_map<koopa_raw_value_t, int> integer_map;

string printBBName(const string & str){
  return str.substr(1);
}

inline bool is_global(const koopa_raw_value_t &v) {
  return v->kind.tag == KOOPA_RVT_GLOBAL_ALLOC;
}

int calSize(const koopa_raw_type_t&type){
  switch (type->tag) {
    case KOOPA_RTT_INT32:
      return 4;
    case KOOPA_RTT_POINTER:
      return 4; // SysY / Koopa: 32-bit pointer
    case KOOPA_RTT_ARRAY:
      return type->data.array.len * calSize(type->data.array.base);
    default:
      assert(false && "unknown koopa type");
  }
}


void EmitArrayInitialization(const koopa_raw_value_t &value){
  if(value->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  .word " << value->kind.data.integer.value << endl;
  }else if(value->kind.tag == KOOPA_RVT_ZERO_INIT){
    // do nothing here
    cout << "  .zero " << calSize(value->ty) << endl;  
  }else if(value->kind.tag == KOOPA_RVT_AGGREGATE){
    auto &agg_init = value->kind.data.aggregate;
    for(int i = 0; i < agg_init.elems.len; ++i){
      koopa_raw_value_t elem = reinterpret_cast<koopa_raw_value_t>(agg_init.elems.buffer[i]);
      EmitArrayInitialization(elem);
    }
  }else{
    cerr << "Error: Unsupported array initialization kind tag: " << value->kind.tag << endl;
    assert(false);
  }
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

  int off = sp_gap - 4;
  if (is_function_called) {
    if (off >= -2048 && off <= 2047) {
      cout << "  lw ra, " << off << "(sp)\n";
    } else {
      cout << "  li t0, " << off << "\n";
      cout << "  add t0, sp, t0\n";
      cout << "  lw ra, 0(t0)\n";
    }
  }

  // return to the previous stack frame
  if(sp_gap > 2047){
    cout << "  li t0, " << sp_gap << endl;
    cout << "  add sp, sp, t0" << endl;
  }else if(sp_gap <= 2047 && sp_gap > 0) {
    cout << "  addi sp, sp, " << sp_gap << endl;
  }
  cout << "  ret" << endl;
  cout << endl;
}

void Visit(const koopa_raw_global_alloc_t &global_alloc, const koopa_raw_value_t &value){
  cout << "  .data" << endl;
  string tmp(value->name);
  tmp = tmp.substr(1);
  cout << tmp << ":" << endl;

  EmitArrayInitialization(global_alloc.init);
  cout << endl;
}

void Visit(const koopa_raw_func_arg_ref_t &func_arg_ref, const koopa_raw_value_t &value){
  // do nothing here
}

void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr, const koopa_raw_value_t &value){
  if(is_global(get_elem_ptr.src)){
    string tmp(get_elem_ptr.src->name);
    cout << "  la t0, " << tmp.substr(1) << endl;
  } else {
    int src_offset = stack_offset_map[get_elem_ptr.src];
    // 判断 src 是否是之前的指针计算结果
    if (get_elem_ptr.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || 
        get_elem_ptr.src->kind.tag == KOOPA_RVT_GET_PTR) {
      // 如果 src 是指针，从栈里加载这个地址值
      LoadFromStack("t0", src_offset);
    } else {
      // 如果 src 是 alloc 出来的数组，计算其在栈上的首地址
      if (src_offset > 2047 || src_offset < -2048) {
        cout << "  li t0, " << src_offset << endl;
        cout << "  add t0, sp, t0" << endl;
      } else {
        cout << "  addi t0, sp, " << src_offset << endl;
      }
    }
  }

  if(get_elem_ptr.index->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  li t1, " << get_elem_ptr.index->kind.data.integer.value << endl;
  }else{
    LoadFromStack("t1", stack_offset_map[get_elem_ptr.index]);
  }

  cout << "  li t2, " << calSize(get_elem_ptr.src->ty->data.pointer.base->data.array.base) << endl;
  cout << "  mul t1, t1, t2" << endl;
  cout << "  add t0, t0, t1" << endl;
  stack_offset_map[value] = offset*4;
  StoreToStack("t0", stack_offset_map[value]);
  offset++;
  return;
}


void Visit(const koopa_raw_get_ptr_t &get_ptr, const koopa_raw_value_t &value){
  if(is_global(get_ptr.src)){
    string tmp(get_ptr.src->name);
    cout << "  la t0, " << tmp.substr(1) << endl;
  } else {
    int src_offset = stack_offset_map[get_ptr.src];
    // 判断 src 是否是之前的指针计算结果
    if (get_ptr.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || 
        get_ptr.src->kind.tag == KOOPA_RVT_GET_PTR) {
      // 如果 src 是指针，从栈里加载这个地址值
      LoadFromStack("t0", src_offset);
    } else {
      // 如果 src 是 alloc 出来的数组，计算其在栈上的首地址
      if (src_offset > 2047 || src_offset < -2048) {
        cout << "  li t0, " << src_offset << endl;
        cout << "  add t0, sp, t0" << endl;
      } else {
        cout << "  addi t0, sp, " << src_offset << endl;
      }
    }
  }

  if(get_ptr.index->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  li t1, " << get_ptr.index->kind.data.integer.value << endl;
  }else{
    LoadFromStack("t1", stack_offset_map[get_ptr.index]);
  }
  cout << "  li t2, " << calSize(get_ptr.src->ty->data.pointer.base) << endl;
  cout << "  mul t1, t1, t2" << endl;
  cout << "  add t0, t0, t1" << endl;
  stack_offset_map[value] = offset*4;
  StoreToStack("t0", stack_offset_map[value]);
  offset++;
  return;
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
  // check if this is global allocate, if yes then we don't need to generate the initialization again.
  if(load.src->kind.tag == KOOPA_RVT_GET_PTR || load.src->kind.tag  == KOOPA_RVT_GET_ELEM_PTR){
    Visit(load.src);
    LoadFromStack("t0", stack_offset_map[load.src]);
    cout << "  lw t0, 0(t0)" << endl;
    StoreToStack("t0", stack_offset_map[value]);
    return;
  }
  if(load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
    // need to load from the global definition area.
    string tmp(load.src->name);
    tmp = tmp.substr(1);
    cout << "  la t0, " << tmp << endl;
    cout << "  lw t0, 0(t0)" << endl;
    StoreToStack("t0", stack_offset_map[value]);
    return;
  }
  Visit(load.src);
  LoadFromStack("t0", stack_offset_map[load.src]);
  StoreToStack("t0", stack_offset_map[value]);
}

void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value){
  Visit(store.value);
  
  // handle global variable, global var shouldn't visit
  // visit twice will generate extra definition for global var
  if(store.dest->kind.tag != KOOPA_RVT_GLOBAL_ALLOC){
    Visit(store.dest);
  }
  // pure constant integer
  // cerr << store.value->kind.data.integer.value << endl;
  // cerr << offset << endl;

  // for loading parameters from registers
  if(stack_offset_map.count(store.value) && stack_offset_map[store.value] < 0) { // load from register
    StoreToStack("a" + to_string(- stack_offset_map[store.value] - 1), stack_offset_map[store.dest]);
    return;
  }

  // for loading extra parameters from last stack frame,
  // need to add current stack frame size and the offset.
  // if they havfe same name like %x = @x, which is loading the parameters, we should compare and add
  // a gap, which is the current total_variable_number.
  // 
  // check exist before use. sometimes the store value is a integer without name.
  // if(store.value->name && store.dest->name){
  //   string a((store.value->name));
  //   string b((store.dest->name));
  //   if(a.substr(1) == b.substr(1)){
  //     stack_offset_map[store.value] = total_variable_number + stack_offset_map[store.value];
  //   }
  // }

  // normal store command
  if(store.value->kind.tag == KOOPA_RVT_INTEGER){
    cout << "  li t0, " << store.value->kind.data.integer.value << endl;
  }else{
    LoadFromStack("t0", stack_offset_map[store.value]);
  }

  if(store.dest->kind.tag != KOOPA_RVT_GET_PTR && store.dest->kind.tag != KOOPA_RVT_GET_ELEM_PTR){
    // if store dest is a global alloc, we should read the address
    if(store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
      string tmp((store.dest->name));
      tmp = tmp.substr(1);
      cout << "  la t1, " << tmp << endl; 
      cout << "  sw t0, 0(t1)" << endl;
    }else{
      StoreToStack("t0", stack_offset_map[store.dest]);
    }
  }else{
    // for the pointer value 
    LoadFromStack("t1", stack_offset_map[store.dest]);
    cout << "  sw t0, 0(t1)" << endl;
  }
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
  if(value->ty->tag != KOOPA_RTT_UNIT){
    stack_offset_map[value] = offset*4;
    offset++;
    StoreToStack("a0", stack_offset_map[value]);
  }

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
        offset += calSize(value->ty->data.pointer.base) / 4;
        break;
      case KOOPA_RVT_LOAD:
        stack_offset_map[value] = offset*4;
        offset++;
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
      case KOOPA_RVT_GLOBAL_ALLOC:
        Visit(kind.data.global_alloc, value);
        break;
      case KOOPA_RVT_GET_PTR:
        Visit(kind.data.get_ptr, value);
        break;
      case KOOPA_RVT_GET_ELEM_PTR:
        Visit(kind.data.get_elem_ptr, value);
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
      // skip the duplicated definition.
      if (func->bbs.len == 0) {
        return;
      }

    // get the function name.
    string funcName = string(func->name).substr(1);

    // find the function names.
    if (func_total_vars_map.find(funcName) == func_total_vars_map.end()) {
        cerr << "Error: Metadata not found for function " << funcName << endl;
        return; 
    }

    total_variable_number = func_total_vars_map[funcName];
    max_parameter_number = func_max_params_map[funcName];
    is_function_called = func_is_called_map[funcName];

    // offset means the start position to arrange the temporary variables
    offset = max(max_parameter_number - 8,0);
    stack_offset_map.clear();
    visited.clear();
    //integer_map.clear();


    cout << "  .globl " << funcName << endl;  // skip the '@' character
    cout << funcName << ":" << endl;
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

    int off = sp_gap - 4;
    if (is_function_called) {
      if (off >= -2048 && off <= 2047) {
        cout << "  sw ra, " << off << "(sp)\n";
      } else {
        cout << "  li t0, " << off << "\n";
        cout << "  add t0, sp, t0\n";
        cout << "  sw ra, 0(t0)\n";
      }
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
    // Visit(program.values);
    for(int i = 0; i < program.values.len; ++i){
      koopa_raw_value_t var = reinterpret_cast<koopa_raw_value_t>(program.values.buffer[i]);
      Visit(var);
    }
    cout << "  .text" << endl;
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
  