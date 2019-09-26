# SimpleLanguageParser
Parser for language, look like C/C++


## Algorithm of this parser
&emsp;**Input Text**&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;*(written with my own syntax)*

       |
      \|
**Lexical Analyzer**&emsp;&emsp;&emsp;&emsp;&emsp;*(devide text on `lexems` and check all allowed symbols)*

      |
     \|
**List of Lexems**&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;*( `intermediate` representation of input lexems as json list)*

      |
     \|
**Syntactic Analyzer**&emsp;&emsp;&emsp;&emsp;*(check on valid syntax and build `AST tree` as result)*

      |
     \| 
&emsp;&emsp;**AST Tree**&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;*( `intermediate` representation of input program)*\
   
      |
     \|
&emsp;&emsp;**Reverse**\
**Polish Notation**&emsp;&emsp;&emsp;&emsp;&emsp;*(well, no comments)*\
&emsp;**and Executing**


## Grammar of My Language in EBNF format (GorodLang)
```ebnf
Gorod = Block .

Block = "{"       
            {VarDefinition}
            {Statement} 
        "}" .

VarDefinition = Type Ident {"," Ident} ";" .

Type = "int" .

Ident = letter {letter | digit | "_"} .

Statement = [Assignment] ";"
            | "read" "(" Ident ")" ";"
            | "write" "(" Expr ")" ";"
            | "if" "(" LogicExpr ")" Block 
            | "for" Assignment "to" Expr "by" Expr "while" "(" LogicExpr ")" Block "rof" ";" . 

Assignment = Ident "=" Expr .

Expr = CondExpr
       | Add . 

Add = Mult [( "+" | "-" ) Mult] .

Mult = Power [( "*" | "/" ) Power] .

Power = Group ["^" Power] .

Group = "(" Expr ")"  
        | Number
        | Ident .

CondExpr = LogicExpr ["?" Expr ":" Expr] .

LogicExpr = Ident ("<" | ">") Expr .

Number = [ "-" ] digit{digit} .
```

## Examle of input program
```c
{
  int k; int sum, input, i;
  sum = 0;
  for i=0; to 3^144 by k+1 while(sum<1000){
		    read(input);
		  if (input > 0)
		  {
		    sum = sum + input;
		    write(sum);
		  }
		
	}rof;
  sum = sum^2;
  write(sum);
}
```

## Example of lexical analyzer result (json table of lexems)
```json
[
    {
        "line": 1,
        "posInLine": 1,
        "type": "token",
        "value": "{"
    },
    {
        "line": 2,
        "posInLine": 3,
        "type": "token",
        "value": "int"
    },
    {
        "id": 0,
        "line": 2,
        "posInLine": 7,
        "type": "ident",
        "value": "k"
    },
    
    ........
```
[more here](/docs/LexerTable.csv)

## Example of AST tree after Syntactical anal
[ASTree realization](https://github.com/Childcity/ASTree)
![ast_tree](/docs/prog.gor_ASTree.png)
