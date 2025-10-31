
THE KLEENE PROGRAMMING LANGUAGE
===============================

Kleene, inspired by [recursive function theory](https://en.wikipedia.org/wiki/General_recursive_function), is a programming language based on (partial-) recursive functions.

All functions have domains as finite dimensional vectors on ***N***,  where ***N*** is the set of all natural numbers starting from 0. The range is always ***N***.

There are three classes of primitve functions:

+ ***C<sup>n</sup><sub>k<sub>***, the *n*-ary function that produces constant natural number *k*.
+ ***P<sup>n</sup><sub>k<sub>***, the projection on *k*-th dimension.
+ ***S***, the successor function. If input *x*, it produces *x+1*.

And there are three ways to build up custom functions:

+ Composition. 
+ Primitive recursion.
+ Minimization.

Examples:
***C<sup>2</sup><sub>9<sub>**(x<sub>1</sub>,x<sub>2</sub>) = 9*. ***P<sup>3</sup><sub>1<sub>**(x<sub>1</sub>,x<sub>2</sub>,x<sub>3</sub>) = x<sub>1</sub>*. ***S** 3=4*.

### Syntax
```bnf
<program> = <line> {'\n'+ <line>}*
<line> = <variable> '=' <expression> [';' <comment>]
<expression> = <identifer> '(' <identifer> {',' <identifer>}* ')' 
             | <identifer> '@' <identifer>
             | '$' <identifer>
<identifer> = 'C^'<num>'_'<num> | 'P^'<num>'_'<num> | 'S' | <variable>
<variable> = {'a' | ... | 'z'}{'A' | ... | 'Z' | 'a' | ... | 'z' | '0' | '1' | ... | '9'}*
<comment> = {any character except newline}*
```
White space (spaces and tabs) can appear between any two tokens and should be ignored.