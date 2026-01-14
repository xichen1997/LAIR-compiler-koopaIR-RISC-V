%locations
%debug

%define api.pure
%define api.push-pull pull

%code requires {
  #include <memory>
  #include <string>
  #include "../include/AST.h"
}

%code{
// 声明 lexer 函数和错误处理函数
// int yylex();
int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);
void yyerror(YYLTYPE *loc, std::unique_ptr<BaseAST> &ast, const char *s);
}

%{

#include <iostream>
#include <memory>
#include <string>


using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT PLUS MINUS NOT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> Block Stmt Number 
// 定义运算相关
%type <ast_val> FuncDef FuncType Exp PrimaryExp UnaryExp
// 
%type <str_val> UnaryOp

%%
// 把所有的东西都放到类里面，用unique pointer 管理各个ast。
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnit>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    comp_unit->lineno = @1.first_line;
    ast = std::move(comp_unit);
    cerr << "[AST] Built CompUnit at line " << @1.first_line << endl;
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDef();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->lineno = @2.first_line; 
    $$ = ast; 
    cerr << "[AST] Built FuncDef at line " << @2.first_line << endl;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncType();
    ast->functype = make_unique<string>("int");
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built FuncType at line " << @1.first_line << endl;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new Block();
    ast->stmt = unique_ptr<BaseAST>($2);
    ast->lineno = @1.first_line;  
    $$ = ast;
    cerr << "[AST] Built Block at line " << @1.first_line << endl;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto exp = dynamic_cast<Exp*>($2);
    auto ast = new Stmt();
    ast->exp = unique_ptr<Exp>(exp);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  ;

Number
  : INT_CONST {
    auto ast = new Number();
    ast->int_const = $1;
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built Number at line " << @1.first_line << endl;
  }
  ;

Exp 
  : UnaryExp {
    auto ue = dynamic_cast<UnaryExp*>($1);
    auto ast = new Exp();
    ast->unary_exp = unique_ptr<UnaryExp>(ue);
    ast->lineno = @1.first_line; 
    $$ = ast; 
    cerr << "[AST] Built Exp at line " << @1.first_line << endl;
  }
  ;

PrimaryExp
  : '(' Exp ')' { 
    auto e = dynamic_cast<Exp*>($2); 
    auto ast = new PrimaryExp(); 
    ast->kind = PrimaryExp::_Exp; 
    ast->exp = unique_ptr<Exp>(e); 
    ast->lineno = @1.first_line; 
    $$ = ast; 
    cerr << "[AST] Built PrimaryExp at line " << @1.first_line << endl;
  }
  | Number { 
    auto num = dynamic_cast<Number*>($1);
    auto ast = new PrimaryExp(); 
    ast->kind = PrimaryExp::_Number;
    ast->number = std::unique_ptr<Number>(num);
    ast->lineno = @1.first_line; 
    $$ = ast; 
    cerr << "[AST] Built PrimaryExp at line " << @1.first_line << endl;
  }
  ;

UnaryExp
  : PrimaryExp { 
    auto pe = dynamic_cast<PrimaryExp*>($1);
    auto ast = new UnaryExp(); 
    ast->kind = UnaryExp::_PrimaryExp; 
    ast->primary_exp = unique_ptr<PrimaryExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built UnaryExp at line " << @1.first_line << endl;
  }
  | UnaryOp UnaryExp {
    auto op = $1;
    auto ue = dynamic_cast<UnaryExp*>($2);

    auto ast = new UnaryExp(); 
    ast->kind = UnaryExp::_UnaryOp_UnaryExp; 

    ast->unary_op.reset(op);
    ast->unary_exp.reset(ue);

    ast->lineno = @1.first_line; 
    $$ = ast; 
    cerr << "[AST] Built UnaryExp at line " << @1.first_line << endl;
  }
  ;


UnaryOp
  : PLUS   { $$ = $1; }
  | MINUS  { $$ = $1; }
  | NOT    { $$ = $1; }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(YYLTYPE *loc, std::unique_ptr<BaseAST> &ast, const char *s) {
    std::cerr << "Syntax error: " << s
              << " at line " << loc->first_line
              << std::endl;
}