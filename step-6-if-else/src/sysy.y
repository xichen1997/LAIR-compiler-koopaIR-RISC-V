%locations
%debug

%define api.pure
%define api.push-pull pull

%code requires {
  #include <memory>
  #include <string>
  #include <vector>
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
%token INT RETURN CONST IF ELSE
%token <str_val> IDENT PLUS MINUS NOT MUL DIV MOD LE GE LESS GREATER SAME NOTSAME LAND LOR 
%token <int_val> INT_CONST 

// 非终结符的类型定义
%type <ast_val> Block BlockItemList BlockItem Stmt Number
// 定义运算相关
%type <ast_val> Decl FuncDef FuncType Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp ConstExp
// 定义变量常量
%type <ast_val> ConstDecl ConstDefList ConstDef ConstInitVal VarDecl VarDefList VarDef InitVal
// 接近于终结符，但是可以有多种表示形式，比如各种运算
%type <str_val> UnaryOp MulOp AddOp RelOp EqOp LAndOp LOrOp LVAL BType

// 定义优先级
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

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

BType
  : INT{
    $$ = new string("INT");
  }
  ;

Decl
  : ConstDecl {
    auto cd = dynamic_cast<ConstDecl*>($1);
    auto ast = new Decl();
    ast->kind = Decl::_ConstDecl;
    ast->const_decl.reset(cd);
    $$ = ast;
    cerr << "[AST] Built Decl at line " << @1.first_line << endl;
  }
  | VarDecl {
    auto vd = dynamic_cast<VarDecl*>($1);
    auto ast = new Decl();
    ast->kind = Decl::_VarDecl;
    ast->var_decl.reset(vd);
    $$ = ast; 
    cerr << "[AST] Built Decl at line " << @1.first_line << endl;
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';'{
    auto bt = $2;
    auto cdl = dynamic_cast<ConstDefList*>($3);
    auto ast = new ConstDecl();
    ast->btype.reset(bt);
    ast->const_def_list.reset(cdl);
    $$ = ast;
    cerr << "[AST] Built ConstDecl at line " << @1.first_line << endl;
  }
  ;

ConstDefList
  : ConstDef {
    auto cd = dynamic_cast<ConstDef*>($1);
    auto ast = new ConstDefList();
    ast->const_defs.emplace_back(cd);
    $$ = ast;
    cerr << "[AST] Built ConstDefList at line " << @1.first_line << endl;
  }
  | ConstDefList ',' ConstDef{
    auto cd = dynamic_cast<ConstDef*>($3);
    auto cdl = dynamic_cast<ConstDefList*>($1);
    cdl->const_defs.emplace_back(cd);
    $$ = cdl;
    cerr << "[AST] Built ConstDefList at line " << @1.first_line << endl;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto civ = dynamic_cast<ConstInitVal*>($3);
    auto ast = new ConstDef();
    ast->const_init_val.reset(civ);
    ast->ident.reset($1);
    $$ = ast;
    cerr << "[AST] Built ConstDef at line " << @1.first_line << endl;
  }
  ;

VarDecl
  : BType VarDefList ';'{
    auto bt = $1;
    auto vdl = dynamic_cast<VarDefList*>($2);
    auto ast = new VarDecl();
    ast->btype.reset(bt);
    ast->var_def_list.reset(vdl);
    $$ = ast;
    cerr << "[AST] Built VarDecl at line " << @1.first_line << endl;
  }
  ;

VarDefList
  : VarDef {
    auto ast = new VarDefList();
    auto vd = dynamic_cast<VarDef*>($1);
    ast->var_defs.emplace_back(vd);
    $$ = ast;
    cerr << "[AST] Built VarDefList at line " << @1.first_line << endl;
  }
  | VarDefList ',' VarDef {
    auto vdl = dynamic_cast<VarDefList*>($1);
    auto vd = dynamic_cast<VarDef*>($3);
    vdl->var_defs.emplace_back(vd);
    $$ = vdl;
    cerr << "[AST] Built VarDefList at line " << @1.first_line << endl;
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDef();
    ast->ident.reset($1);
    $$ = ast;
    cerr << "[AST] Built VarDef at line " << @1.first_line << endl;
  }
  | IDENT '=' InitVal { 
    auto ast = new VarDef();
    auto iv = dynamic_cast<InitVal*>($3);
    ast->ident.reset($1);
    ast->init_val.reset(iv);
    $$ = ast;
    cerr << "[AST] Built VarDef at line " << @1.first_line << endl;
  }
  ;

Block
  : '{' BlockItemList '}' {
    auto bil = dynamic_cast<BlockItemList*>($2);
    auto ast = new Block();
    ast->block_item_list.reset(bil);
    ast->lineno = @1.first_line;  
    $$ = ast;
    cerr << "[AST] Built Block at line " << @1.first_line << endl;
  }
  ;

BlockItemList
  : /* empty */{
    $$ = new BlockItemList();
  }
  | BlockItemList BlockItem{
    auto bi = dynamic_cast<BlockItem*>($2);
    auto bil = dynamic_cast<BlockItemList*>($1);
    bil->block_items.emplace_back(bi);
    bil->lineno = @1.first_line;
    $$ = bil;
    cerr << "[AST] Built BlockItemList at line " << @1.first_line << endl;
  }
  ;

BlockItem
  : Decl {
    auto decl = dynamic_cast<Decl*>($1);
    auto ast = new BlockItem();
    ast->kind = BlockItem::_Decl;
    ast->decl.reset(decl);
    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built BlockItem at line " << @1.first_line << endl;
  }
  | Stmt {    
    auto stmt = dynamic_cast<Stmt*>($1);
    auto ast = new BlockItem();
    ast->kind = BlockItem::_Stmt;
    ast->stmt.reset(stmt);
    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built BlockItem at line " << @1.first_line << endl;
  }
  ;

Stmt
  : LVAL '=' Exp ';'{
    auto exp = dynamic_cast<Exp*>($3);
    auto ast = new Stmt();
    ast->kind = Stmt::_Lval_Assign_Exp;
    ast->lval.reset($1);
    ast->exp.reset(exp);
    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  | RETURN Exp ';' {
    auto exp = dynamic_cast<Exp*>($2);
    auto ast = new Stmt();
    ast->kind = Stmt::_Return_Exp;
    ast->exp = unique_ptr<Exp>(exp);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  | Exp ';' {
    auto exp = dynamic_cast<Exp*>($1);
    auto ast = new Stmt();
    ast->kind = Stmt::_Exp;
    ast->exp.reset(exp);
    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  | ';' {
    auto ast = new Stmt();
    ast->kind = Stmt::_Empty;
    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  | Block {
    auto b = dynamic_cast<Block*>($1);
    auto ast = new Stmt();
    ast->kind = Stmt::_Block;
    ast->block.reset(b);
    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  | IF '(' Exp ')' Stmt %prec LOWER_THAN_ELSE{
    auto ast = new Stmt();
    auto e = dynamic_cast<Exp*>($3);
    auto ifstmt = dynamic_cast<Stmt*>($5);
    ast->kind = Stmt::_If_Stmt;
    ast->exp.reset(e);
    ast->ifstmt.reset(ifstmt);
    $$ = ast;
    cerr << "[AST] Built Stmt at line " << @1.first_line << endl;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt{
    auto ast = new Stmt();
    auto e = dynamic_cast<Exp*>($3);
    auto ifstmt = dynamic_cast<Stmt*>($5);
    auto elsestmt = dynamic_cast<Stmt*>($7);
    ast->kind = Stmt::_If_Stmt_Else_Stmt;
    ast->exp.reset(e);
    ast->ifstmt.reset(ifstmt);
    ast->elsestmt.reset(elsestmt);
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
  : LOrExp {
    auto ue = dynamic_cast<LOrExp*>($1);
    auto ast = new Exp();
    ast->lor_exp = unique_ptr<LOrExp>(ue);
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
  | LVAL {
    auto ast = new PrimaryExp();
    ast->kind = PrimaryExp::_Lval;
    ast->ident.reset($1);
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

MulExp
  : UnaryExp {
    auto pe = dynamic_cast<UnaryExp*>($1);
    auto ast = new MulExp(); 
    ast->kind = MulExp::_UnaryExp; 
    ast->unary_exp = unique_ptr<UnaryExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built MulExp at line " << @1.first_line << endl;
  }
  | MulExp MulOp UnaryExp {
    auto lval = dynamic_cast<MulExp*>($1);
    auto op = $2;
    auto rval = dynamic_cast<UnaryExp*>($3);
    
    auto ast = new MulExp();
    ast->kind = MulExp::_MulExp_MulOp_UnaryExp;

    ast->mul_exp.reset(lval);
    ast->mul_op.reset(op);
    ast->unary_exp.reset(rval);

    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built MulExp at line " << @1.first_line << endl;
  }
  ;

AddExp 
  : MulExp {
    auto pe = dynamic_cast<MulExp*>($1);
    auto ast = new AddExp(); 
    ast->kind = AddExp::_MulExp; 
    ast->mul_exp = unique_ptr<MulExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built AddExp at line " << @1.first_line << endl;
  }
  | AddExp AddOp MulExp{
    auto lval = dynamic_cast<AddExp*>($1);
    auto op = $2;
    auto rval = dynamic_cast<MulExp*>($3);

    auto ast = new AddExp();
    ast->kind = AddExp::_AddExp_AddOp_MulExp;

    ast->add_exp.reset(lval);
    ast->add_op.reset(op);
    ast->mul_exp.reset(rval);

    ast->lineno = @1.first_line;
    $$ = ast;
    cerr << "[AST] Built AddExp at line " << @1.first_line << endl;
  }
  ;

RelExp
  : AddExp {
    auto pe = dynamic_cast<AddExp*>($1);
    auto ast = new RelExp(); 
    ast->kind = RelExp::_AddExp; 
    ast->add_exp = unique_ptr<AddExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built RelExp at line " << @1.first_line << endl;
  }
  | RelExp RelOp AddExp {
    auto lval = dynamic_cast<RelExp*>($1);
    auto op = $2;
    auto rval = dynamic_cast<AddExp*>($3);
    
    auto ast = new RelExp();
    ast->kind = RelExp::_RelExp_RelOp_AddExp;

    ast->rel_exp.reset(lval);
    ast->rel_op.reset(op);
    ast->add_exp.reset(rval);

    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built RelExp at line " << @1.first_line << endl;
  }
  ;


EqExp
  : RelExp {
    auto pe = dynamic_cast<RelExp*>($1);
    auto ast = new EqExp(); 
    ast->kind = EqExp::_RelExp; 
    ast->rel_exp = unique_ptr<RelExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built EqExp at line " << @1.first_line << endl;
  }
  | EqExp EqOp RelExp {
    auto lval = dynamic_cast<EqExp*>($1);
    auto op = $2;
    auto rval = dynamic_cast<RelExp*>($3);
    
    auto ast = new EqExp();
    ast->kind = EqExp::_EqExp_EqOp_RelExp;

    ast->eq_exp.reset(lval);
    ast->eq_op.reset(op);
    ast->rel_exp.reset(rval);

    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built EqExp at line " << @1.first_line << endl;
  }
  ;

LAndExp
  : EqExp {
    auto pe = dynamic_cast<EqExp*>($1);
    auto ast = new LAndExp(); 
    ast->kind = LAndExp::_EqExp; 
    ast->eq_exp = unique_ptr<EqExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built LAndExp at line " << @1.first_line << endl;
  }
  | LAndExp LAndOp EqExp {
    auto lval = dynamic_cast<LAndExp*>($1);
    auto op = $2;
    auto rval = dynamic_cast<EqExp*>($3);
    
    auto ast = new LAndExp();
    ast->kind = LAndExp::_LAndExp_LAndOp_EqExp;

    ast->land_exp.reset(lval);
    ast->land_op.reset(op);
    ast->eq_exp.reset(rval);

    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built LAndExp at line " << @1.first_line << endl;
  }
  ;

LOrExp
  : LAndExp {
    auto pe = dynamic_cast<LAndExp*>($1);
    auto ast = new LOrExp(); 
    ast->kind = LOrExp::_LAndExp; 
    ast->land_exp = unique_ptr<LAndExp>(pe);
    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built LOrExp at line " << @1.first_line << endl;
  }
  | LOrExp LOrOp LAndExp {
    auto lval = dynamic_cast<LOrExp*>($1);
    auto op = $2;
    auto rval = dynamic_cast<LAndExp*>($3);
    
    auto ast = new LOrExp();
    ast->kind = LOrExp::_LOrExp_LOrOp_LAndExp;

    ast->lor_exp.reset(lval);
    ast->lor_op.reset(op);
    ast->land_exp.reset(rval);

    ast->lineno = @1.first_line; 
    $$ = ast;
    cerr << "[AST] Built LOrExp at line " << @1.first_line << endl;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ce = dynamic_cast<ConstExp*>($1);
    auto ast = new ConstInitVal();
    ast->const_exp.reset(ce);
    $$ = ast;
    cerr << "[AST] Built ConstInitVal at line " << @1.first_line << endl;
  }
  ;


InitVal
  : Exp{
    auto e = dynamic_cast<Exp*>($1);
    auto ast = new InitVal();
    ast->exp.reset(e);
    $$ = ast;
    cerr << "[AST] Built InitVal at line " << @1.first_line << endl;
  }
  ;


ConstExp
  : Exp {
    auto e = dynamic_cast<Exp*>($1);
    auto ast = new ConstExp();
    ast->exp.reset(e);
    $$ = ast;
    cerr << "[AST] Built ConstExp at line " << @1.first_line << endl;
  }
  ;

UnaryOp
  : PLUS   { $$ = $1; }
  | MINUS  { $$ = $1; }
  | NOT    { $$ = $1; }
  ;

MulOp
  : MUL    { $$ = $1; }
  | DIV    { $$ = $1; }
  | MOD    { $$ = $1; }
  ;

AddOp
  : PLUS   { $$ = $1; }
  | MINUS  { $$ = $1; }
  ;

RelOp
  : LE     { $$ = $1; }
  | GE     { $$ = $1; }
  | LESS   { $$ = $1; }
  | GREATER{ $$ = $1; }
  ;

EqOp
  : SAME   { $$ = $1; }
  | NOTSAME{ $$ = $1; }
  ;

LAndOp 
  : LAND   { $$ = $1; }

LOrOp
  : LOR    { $$ = $1; }

LVAL
  : IDENT  { $$ = $1; }
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(YYLTYPE *loc, std::unique_ptr<BaseAST> &ast, const char *s) {
    std::cerr << "Syntax error: " << s
              << " at line " << loc->first_line
              << std::endl;
}