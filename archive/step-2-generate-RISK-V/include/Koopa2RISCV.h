#pragma once
#include <cassert>
#include <iostream>
#include "koopa.h"

using namespace std;

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);

inline void Visit(const koopa_raw_integer_t &integer) { cout << integer.value; }

inline void Visit(const koopa_raw_return_t &ret) {
  if (ret.value != nullptr) {
    cout << "  li a0, ";
    Visit(ret.value);
    cout << endl;
  }
  cout << "  ret" << endl;
}

inline void Visit(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      Visit(kind.data.integer);
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
