#pragma once
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "koopa.h"

using namespace std;

int register_count = 0;
unordered_map<koopa_raw_value_t, string> temp_register_map; // 用于存储临时寄存器名称 第一次见到旧分配t[temp_register_map] = "t0" 第二次见到temp_register_map[t] = "t1"
unordered_set<koopa_raw_value_t> visited; // 防止重复访问

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);

inline void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value) {
  if(integer.value == 0){
    // use zero register directly
    temp_register_map[value] = "x0";
    return;
  }
  temp_register_map[value] = "t" + to_string(register_count++);
  cout << "  li " << temp_register_map[value] << ", " << integer.value << endl;
 }

inline void Visit(const koopa_raw_return_t &ret) {
  if (ret.value != nullptr) {
    string src = temp_register_map[ret.value];
    cout << "  mv a0, " << src << "\n";
  }
  cout << "  ret" << endl;
}

inline void Visit(const koopa_raw_value_t &value) {

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
    default:
      assert(false);
  }
}

inline void Visit(const koopa_raw_basic_block_t &bb) { Visit(bb->insts); }

inline void Visit(const koopa_raw_function_t &func) {
  cout << "  .globl " << func->name + 1 << endl;  // skip the '@' character
  cout << func->name + 1 << ":" << endl;
  Visit(func->bbs);
}

inline void Visit(const koopa_raw_slice_t &slice) {
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

inline void Visit(const koopa_raw_program_t &program) {
  cout << "  .text" << endl;
  Visit(program.values);
  Visit(program.funcs);
}

inline void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value) {

  Visit(binary.lhs);
  Visit(binary.rhs);

  temp_register_map[value] = "t" + to_string(register_count++);

  string lhs = temp_register_map[binary.lhs];
  string rhs = temp_register_map[binary.rhs];

  switch (binary.op){
    case KOOPA_RBO_SUB:
      cout << "  sub " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    case KOOPA_RBO_EQ:
      cout << "  xor " << temp_register_map[value] << ", " <<  lhs << ", " << rhs << endl;
      cout << "  seqz " << temp_register_map[value] << ", " << temp_register_map[value] << endl;
    break;
    case KOOPA_RBO_ADD:
      cout << "  add " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    case KOOPA_RBO_MUL:
      cout << "  mul " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    case KOOPA_RBO_DIV:
      cout << "  div " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    case KOOPA_RBO_NOT_EQ:
      cout << "  xor " << temp_register_map[value] << ", " <<  lhs << ", " << rhs << endl;
      cout << "  snez " << temp_register_map[value] << ", " << temp_register_map[value] << endl;
    break;
    case KOOPA_RBO_LT:
      cout << "  slt " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
      cout << "  seqz " << temp_register_map[value] << ", " << temp_register_map[value] << endl;
    break;
    case KOOPA_RBO_GT:
      cout << "  sgt " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    case KOOPA_RBO_LE:
      cout << "  sgt " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
      cout << "  seqz " << temp_register_map[value] << ", " << temp_register_map[value] << endl;
    break;
    case KOOPA_RBO_GE:
      cout << "  slt " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
      cout << "  seqz " << temp_register_map[value] << ", " << temp_register_map[value] << endl;
    break;
    case KOOPA_RBO_AND:
      cout << "  and " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    case KOOPA_RBO_OR:
      cout << "  or " << temp_register_map[value] << ", " << lhs << ", " << rhs << endl;
    break;
    default:
      assert(false);
  }
}
