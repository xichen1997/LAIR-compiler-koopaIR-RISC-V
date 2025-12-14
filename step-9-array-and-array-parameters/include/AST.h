#pragma once
#include <iostream>
#include <memory>
#include <cassert>
#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "koopa.h"

using namespace std;

extern int temp_count;
extern unordered_map<string, int> const_variable_table;
extern unordered_set<string> initialized_variables;
extern unordered_set<string> delacred_variables;
extern vector<int> total_variable_number_list;

class BaseAST;
class CompUnit;
class CompUnitList;
class CompUnitItem;
class FuncDef;
class FuncType;
class FuncFParam;
class FuncFParams;
class FuncRParams;
class FuncDefList;  
class Decl;
class VarDecl;
class VarDefList;
class VarDef;
class InitVal;
class NestedInitVal;
class ConstDecl;
class ConstDefList;
class NestedConstInitVal;
class ConstDef;
class ArrayIndex;
class ConstInitVal;
class BlockItem;
class Block;
class Stmt;
class Exp;
class ConstExp;
class LOrExp;
class LAndExp;
class EqExp;
class RelExp;
class AddExp;
class MulExp;
class UnaryExp;
class PrimaryExp;
class Number;
class LVAL;

using ConstVal = std::variant<int, float, bool, double>;

class BaseAST{
    public:
    int lineno;
    bool has_return;
    bool has_break;
    bool has_continue;
    virtual ~BaseAST() = default;

    virtual void GenerateIR() = 0;
};

class CompUnit : public BaseAST{
    public:
    std::unique_ptr<CompUnitList> comp_unit_list;

    void GenerateIR() override;
};
// varName used for generating RISCV code.
// other used for generating AST koopa IR.

class CompUnitList : public BaseAST {
    public:
    std::vector<std::unique_ptr<CompUnitItem>> list;

    void GenerateIR() override;
};

class CompUnitItem : public BaseAST {
    public:
    enum Kind{
        _FuncDef,
        _Decl
    } kind;
    std::unique_ptr<FuncDef> func_def;
    std::unique_ptr<Decl> decl;

    void GenerateIR() override;
};


class FuncDef : public BaseAST{
    public:
    std::unique_ptr<FuncType> func_type;
    std::unique_ptr<string> ident;
    std::unique_ptr<BaseAST> block;
    std::unique_ptr<FuncFParams> funcfparams;

    void GenerateIR() override;
};

class FuncType : public BaseAST{
    public:
    std::unique_ptr<string> functype;

    void GenerateIR() override;
};

class FuncRParams : public BaseAST{
    public:
    std::vector<std::unique_ptr<Exp>> list;
    
    void GenerateIR() override;
};

class FuncFParams : public BaseAST{
    public:
    std::vector<std::unique_ptr<FuncFParam>> list;

    void GenerateIR() override;
};

class FuncFParam : public BaseAST{
    public:
    std::unique_ptr<string> type;
    std::unique_ptr<string> ident;
    std::unique_ptr<string> varName;

    void GenerateIR() override;
};

class BlockItemList : public BaseAST{
    public:
    std::vector<std::unique_ptr<BlockItem>> block_items;

    void GenerateIR() override;
};

class BlockItem : public BaseAST{
    public:
    enum Kind{
        _Decl,
        _Stmt
    }kind;
    std::unique_ptr<Decl> decl;
    std::unique_ptr<Stmt> stmt; 

    void GenerateIR() override;
};

class Block : public BaseAST{
    public:
    std::unique_ptr<BlockItemList> block_item_list;

    void GenerateIR() override;
};


class Number : public BaseAST{
    public:
    int int_const;

    void GenerateIR() override;
};

class Exp : public BaseAST{
    public:
    std::unique_ptr<string> varName;
    std::unique_ptr<LOrExp> lor_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class PrimaryExp : public BaseAST{
    public:
    enum Kind{
        _Exp,
        _Lval,
        _Number
    }kind;

    std::unique_ptr<string> varName;
    std::unique_ptr<Exp> exp;
    std::unique_ptr<LVAL> lval;
    std::unique_ptr<string> ident;
    std::unique_ptr<Number> number;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class UnaryExp : public BaseAST{
    public:
    enum Kind{
        _PrimaryExp,
        _UnaryOp_UnaryExp,
        _Func_No_Params,
        _Func_With_Params
    } kind;
    
    std::unique_ptr<string> varName;
    std::unique_ptr<PrimaryExp> primary_exp;
    std::unique_ptr<string> unary_op;
    std::unique_ptr<UnaryExp> unary_exp;
    std::unique_ptr<string> func_name;
    std::unique_ptr<FuncRParams> params;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class Stmt : public BaseAST{
    public:
    enum Kind{
        _Lval_Assign_Exp,
        _Return_Exp,
        _Exp,
        _Empty,
        _Block,
        _If_Stmt,
        _If_Stmt_Else_Stmt,
        _While_Stmt,
        _Break,
        _Continue
    }kind;
    std::unique_ptr<Exp> exp;
    std::unique_ptr<LVAL> lval;
    std::unique_ptr<Block> block;
    std::unique_ptr<Stmt> ifstmt;
    std::unique_ptr<Stmt> elsestmt;
    std::unique_ptr<Stmt> whilestmt;

    void GenerateIR() override;
};


class MulExp : public BaseAST{
    public:
    enum Kind { _UnaryExp, _MulExp_MulOp_UnaryExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<MulExp> mul_exp;
    std::unique_ptr<string> mul_op;
    std::unique_ptr<UnaryExp> unary_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class AddExp : public BaseAST{
    public:
    enum Kind { _MulExp, _AddExp_AddOp_MulExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<AddExp> add_exp;
    std::unique_ptr<string> add_op;
    std::unique_ptr<MulExp> mul_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class RelExp : public BaseAST{
    public:
    enum Kind { _AddExp, _RelExp_RelOp_AddExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<RelExp> rel_exp;
    std::unique_ptr<string> rel_op;
    std::unique_ptr<AddExp> add_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class EqExp : public BaseAST{
    public:
    enum Kind { _RelExp, _EqExp_EqOp_RelExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<EqExp> eq_exp;
    std::unique_ptr<string> eq_op;
    std::unique_ptr<RelExp> rel_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class LAndExp : public BaseAST{
    public:
    enum Kind { _EqExp, _LAndExp_LAndOp_EqExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<LAndExp> land_exp;
    std::unique_ptr<string> land_op;
    std::unique_ptr<EqExp> eq_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class LOrExp : public BaseAST{
    public:
    enum Kind { _LAndExp, _LOrExp_LOrOp_LAndExp } kind;
    std::unique_ptr<string> varName;
    std::unique_ptr<LOrExp> lor_exp;
    std::unique_ptr<string> lor_op;
    std::unique_ptr<LAndExp> land_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class ConstExp : public BaseAST{
    public:
    std::unique_ptr<Exp> exp;
    std::unique_ptr<string> varName;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class Decl : public BaseAST{
    public:
    enum Kind{
        _ConstDecl,
        _VarDecl
    }kind;
    std::unique_ptr<VarDecl> var_decl;
    std::unique_ptr<ConstDecl> const_decl;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class ConstDecl : public BaseAST{
    public:
    std::unique_ptr<string> btype;
    std::unique_ptr<ConstDefList> const_def_list;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class ConstDefList : public BaseAST{
    public:
    std::vector<std::unique_ptr<ConstDef>> const_defs;

    void GenerateIR() override;
    void EvaluateConstValues();
};


class ConstDef : public BaseAST{
    public:
    enum Kind{
        _SingleVal,
        _Array
    } kind;
    std::unique_ptr<string> ident;
    std::unique_ptr<ArrayIndex> ai;
    std::unique_ptr<ConstInitVal> const_init_val;

    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class ArrayIndex : public BaseAST{
    public:
    std::vector<std::unique_ptr<ConstExp>> list;

    void GenerateIR() override;
};

class ConstInitVal : public BaseAST{
    public:
    enum Kind{
        _ConstExp,
        _Empty,
        _InitList
    } kind;
    std::unique_ptr<NestedConstInitVal> nested_const_init_val;
    std::unique_ptr<ConstExp> const_exp;
    ConstVal const_value;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class NestedConstInitVal : public BaseAST{
    public:
    std::vector<std::unique_ptr<ConstInitVal>> list;

    void GenerateIR() override;
    void EvaluateConstValues();
};

class VarDecl : public BaseAST{
    public:
    std::unique_ptr<string> btype;
    std::unique_ptr<VarDefList> var_def_list;

    void GenerateIR() override;
};

class VarDefList : public BaseAST{
    public:
    std::vector<std::unique_ptr<VarDef>> var_defs;

    void GenerateIR() override;
};

class VarDef : public BaseAST{
    public:
    enum Kind{
        _SingleVal,
        _InitList
    } kind;
    std::unique_ptr<string> ident;
    std::unique_ptr<ArrayIndex> ai;
    std::unique_ptr<InitVal> init_val;

    void GenerateIR() override;
};

class InitVal : public BaseAST{
    public:
    enum Kind{
        _Exp,
        _InitList,
        _Empty
    } kind;
    std::unique_ptr<Exp> exp;
    std::unique_ptr<NestedInitVal> nested_init_val;
    std::unique_ptr<string> varName;

    void GenerateIR() override;
};

class NestedInitVal : public BaseAST {
    public:
    std::vector<std::unique_ptr<InitVal>> list;

    void GenerateIR() override;
};

class LVAL : public BaseAST{
    public:
    enum Kind {
        _Ident,
        _ArrayElement
    } kind;
    std::unique_ptr<string> ident;
    std::unique_ptr<ArrayIndex> ai;

    void GenerateIR() override;
};