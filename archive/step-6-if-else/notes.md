#
## EBNF form
The added EBNF:
```EBNF
Stmt ::= ...
       | ...
       | ...
       | "if" "(" Exp ")" Stmt ["else" Stmt]
       | ...;
```
But we have to notice that the parser could generate multiple definition use the generator above, like:
```
if(a) if(b) x; else(c) y;
```

cout be explained as:
```c++
if(a){
    if(b) x;
    else y;
}
```
or 
```c++
if(a){
    if(b) x;
}else{
    y;
}
```
Then the affinity of the else should be redefined, here I didn't redefine the non-if-statement method becasue that literally will copy and paste block and Stmt again. Here I use the `%prec` to define the prioriy:

```
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
```

## meet a bug which could lead to AST not being genereated

The added code is like:
```c++

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
  ```

  When I try to compile the code 
  ```c
  int main() {
  if(1){
    if(1) {
      return 0
    }
  }else{
    if(1){
      int a=1;
    }
  }
  return 0;
}
  ```

It give errors about the AST is empty and fail the nullptr check in the `main.cpp`. 


## The code must within some blocks


For example:

This will work
```c++
            // if the condition is true, need to calculate the second block.
            cout << trueLabel << ":"<<endl;
            eq_exp->GenerateIR();
            string rightVar = *(eq_exp->varName);
            cout << "  " << cond2 << " = ne " << rightVar << ", 0" << endl;
            cout << "  store " << cond2 << ", @" << result << endl; 
            cout << "  jump " << endLabel << endl;
            cout << endl;
```

This won't
```c++
            // if the condition is true, need to calculate the second block.
            eq_exp->GenerateIR();
            string rightVar = *(eq_exp->varName);

            cout << trueLabel << ":"<<endl;
            cout << "  " << cond2 << " = ne " << rightVar << ", 0" << endl;
            cout << "  store " << cond2 << ", @" << result << endl; 
            cout << "  jump " << endLabel << endl;
            cout << endl;
```
The calculation of the temp var has been hiding between two basic blocks.


