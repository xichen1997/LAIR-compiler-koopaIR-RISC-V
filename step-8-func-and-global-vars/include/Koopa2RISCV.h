#pragma once
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "koopa.h"

using namespace std;

extern int register_count;
extern unordered_map<koopa_raw_value_t, string> temp_register_map;
extern unordered_set<koopa_raw_value_t> visited;

// 函数声明
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value);
