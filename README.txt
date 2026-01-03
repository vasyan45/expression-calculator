========================
C Expression Calculator
========================

Description:
--------------------
A simple mathematical expression calculator written in C language. Supports
basic arithmetic operations, parentheses, unary operators, and exponentitation.

Features:
--------------------
- Recursive descent parser
- Abstract Syntax Tree (AST) for expression representation
- Separation into lexical analysis, syntax analysis, and interpretation
- Static memory allocation for AST nodes

Expression examples:
--------------------
2+3*4	= 14
(2+3)*4	= 20
-5 + 3	= -2
2^3	= 8
2^3^2	= 512
2*(-3)	= -6

Limitations:
--------------------
- No floating-point number support
- No variable support
- No function support (sin, cos, etc.)
