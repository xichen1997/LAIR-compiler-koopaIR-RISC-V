%option noyywrap
%option nounput
%option noinput
%option yylineno
%option bison-locations

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"
#define YY_USER_ACTION yylloc->first_line = yylineno;

using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*

/* exclusive state */
%x COMMENT  

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+



%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{LineComment}   { /* 忽略, 不做任何操作 */ }

"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }

"const"         { return CONST; }
"int"           { return INT; }
"void"          { return VOID; }
"return"        { return RETURN; }

\n              { yylineno++; }

{Identifier}    { yylval->str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval->int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval->int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval->int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

"!"             { yylval->str_val = new string(yytext); return NOT; }
"+"             { yylval->str_val = new string(yytext); return PLUS; }
"-"             { yylval->str_val = new string(yytext); return MINUS; }
"*"             { yylval->str_val = new string(yytext); return MUL; }
"/"             { yylval->str_val = new string(yytext); return DIV; }
"%"             { yylval->str_val = new string(yytext); return MOD; }  
"<="            { yylval->str_val = new string(yytext); return LE; }
">="            { yylval->str_val = new string(yytext); return GE; }
"<"             { yylval->str_val = new string(yytext); return LESS; }
">"             { yylval->str_val = new string(yytext); return GREATER; }
"=="            { yylval->str_val = new string(yytext); return SAME; }
"!="            { yylval->str_val = new string(yytext); return NOTSAME; }
"&&"            { yylval->str_val = new string(yytext); return LAND; }
"||"            { yylval->str_val = new string(yytext); return LOR; }


"/*"                { BEGIN(COMMENT); }

<COMMENT>"*/"       { BEGIN(INITIAL); }

<COMMENT>[^*\n]+    ;      // 吃掉不是 * 或 换行的部分
<COMMENT>"*"[^/]    ;      // 遇到 * 但不是 */ 的组合
<COMMENT>\n         { yylineno++; }
<COMMENT>"*"        ;      // 单独的 *（无法确定是否是*/

.               { return yytext[0]; }

%%
