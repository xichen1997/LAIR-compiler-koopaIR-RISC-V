#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include "../include/AST.h"
#include "../include/Koopa2RISCV.h"
#include "koopa.h"

extern int total_variable_number;


using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1]; // currently support -koopa and -riscv
  auto input = argv[2];
  auto output = argv[4];

  assert(string(mode) == "-koopa" || string(mode) == "-riscv"); // only support these two modes

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // 关闭输入文件
  fclose(yyin);

  // 把解析出来的AST输出到文件中 
  if(string(mode) == "-koopa"){
      freopen(output, "w", stdout);
      ast->GenerateIR();
      fclose(stdout);
      cerr << total_variable_number << " variables" << endl;
  } else if(string(mode) == "-riscv"){
      freopen("temp.koopa", "w", stdout);
      ast->GenerateIR();
      fclose(stdout);
      // read koopa IR from temp.koopa and store it in const char * str
      ifstream koopa_file("temp.koopa");
      stringstream buffer;
      buffer << koopa_file.rdbuf();
      string koopa_str = buffer.str();
      const char * str = koopa_str.c_str();
      // generate RISC-V code
      // 解析字符串 str, 得到 Koopa IR 程序
      koopa_program_t program;
      koopa_error_code_t ret = koopa_parse_from_string(str, &program);
      assert(ret == KOOPA_EC_SUCCESS);
      koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
      koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
      koopa_delete_program(program);
      // 将 Koopa IR 转换成 RISC-V 汇编代码并输出到文件
      freopen(output, "w", stdout);
      Visit(raw);
      koopa_delete_raw_program_builder(builder);
      fclose(stdout);
  }
  return 0;
}
