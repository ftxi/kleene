
The Kleene Programming Language
===============================

Kleene, inspired by [recursive function theory](https://en.wikipedia.org/wiki/General_recursive_function), is a programming language based on (partial-) recursive functions.

All recursive functions have the type $\mathbb{N}^n\to \mathbb{N}$, i.e. natrual number valued functions on finite-dimensional vectors on $\mathbb{N}$, where $\mathbb{N}$ is the set of all natural numbers starting from 0.

There are three classes of primitve functions:

+ $C^n_k$, the *n*-ary function that produces constant natural number $k$.
+ $P^n_k$, the projection on $k$-th dimension.
+ $S$, the successor function. If input $x$, it produces $x+1$.

And there are three ways to build up custom functions:

+ Composition. If $f:\mathbb{N}^n\to\mathbb{N}$ is recursive and $g_1,g_2,\ldots,g_n$ are functions $\mathbb{N}^k\to\mathbb{N}$, then $h(\vec x)=f(g_1(\vec x),\ldots,g_n(\vec x))$ is recursive. If $n=3$, this is written as `h=f(g1, g2, g3)` in the Kleene language.
+ Primitive recursion. If $f:\mathbb{N}^k\to\mathbb{N}$ and $g:\mathbb{N}^{k+2}\to\mathbb{N}$ are recursive, then so is $h:\mathbb{N}^{k+1}\to\mathbb{N}$ defined by
    - $h(0, \vec x) = f(\vec x)$
    - $h(n+1,\vec x) = g(n,f(n,\vec x), \vec x)$
    - This is written as `h = f @ g`.
+ Minimization. If $g:\mathbb{N}^{k+1}\to\mathbb{N}$ is recursive, then so is $f:\mathbb{N}^k\to\mathbb{N}$ defined as $f(x)$ being the least $n$ such that $g(n,\vec x)=0$ (provided that such $n$ existed for all $x$). This is written as `f = $g`.

If the hypothesis of minimization in parenthesis is removed, the class of functions obtained are called *partial recursive functions*. Becasuse of the difficulties caused by the [Halting problem](https://en.wikipedia.org/wiki/Halting_problem), compile-time checking of whether such $g(x)$ is defined for all $x$ becomes extremely difficult to perform. Therefore, the Kleene language chooses to trust that users will write qualified programs.

Examples:
$C^2_9(x_1, x_2) = 9$, $K^3_1(x_1,x_2,x_3) = x_1$, $S(3) = 4$.

### Syntax

A Kleene program is a sequence of assignments such as
```ini
pred = 0 @ P2_1
minus2 = pred(pred)
```

Let's dive into it.

The first line defines a function `pred` which is the primitive recursion on number $0$ and $P^2_1$. Although $0$ is a number, it is in fact viewed as the $0$-ary constant function $C^0_0$. So the rule of primitive recursion for $k=0$ applies: it defines
+ $\mathrm{pred}(0) = C^0_0 = 0$
+ $\mathrm{pred}(n+1) = P^2_1(n,\mathrm{pred}(n)) = n$

which is exactly $\mathrm{pred}(n) = \max(n-1,0)$.

The name `pred` is chosen as it is short for "predocessor". In principle one can use any names for functions so long as the name starts with a lower letter and consists of upper/lower letters or digits. Any function can be defined only once: one cannot "override" a previously defined function to mean something else.

And the second line is a composition: $\mathrm{minus2}(n)=\mathrm{pred}(\mathrm{pred}(n))=\max(n-2,0)$.

It is possible to write nested expression:

```ini
minus3 = pred(pred(pred))
```
is equivalent to
```ini
minus3 = pred(minus2)
```
The operator `@` and `$` have higher priority than composition. However, `@` and `$` are incomparable; parenthesis are required if you want to use them together.

To write a comment, simply type `;` and everything until end-of-line is ignored:
```ini
; the identity function id(x) = x
id = P1_1
; the function add(x,y) = x+y
add = id @ S(P3_2) ; this is another comment
```

In any expression, you can only refer to primitive functions or functions defined before that line. Self-references are not allowed -- at least not allowed in the obvious way.


### Features

Kleene is strongly typed in the sense that all expressions are dimension-checked so that functions can be composed and applied. For example, `foo = P2_1 @ S` triggers an error:
```
In line 8:
foo = P2_1 @ S
             ^ NEWLINE here
parse error: Dimension mismatch in primitive recursion: S:N^1 -> N does not match P2_1:N^2 -> N
```

The evaluation of expressions in Kleene is short-cuted. For example, in `C1_n(veryComplicated)`, `veryComplicated` is never evaluated no matter what is applied to it. Similarly, for projection on $k$-th axis, only the $k$-th operand is evaluated.


### Try Kleene

Visit <https://ftxi.github.io/kleene/> and try it online!

### Build (Command Line Tool)

Building from source requires a recent c++ compiler and [cmake](https://cmake.org).

Clone/download this repo, go to its base directory and run

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
If everything is okay, try running the Kleene interpreter by
```sh
./kleene meaning_of_life.kl
```
which should calculate 42.

For more information, simply type `./kleene --help`.

### Build (Webassembly)

Ensure that you have [Emscripten](https://emscripten.org) installed. Run the `emcc` command in [compile_ems.sh](compile_ems.sh).

### Formal Definition of Kleene Language

```xml
<program>     ::= <line> {'\n'+ <line>}*
<line>        ::= <variable> '=' <expression> [';' <comment>]
<expression>  ::= <comp-exp> '@' <comp-exp>
                | '$' <comp-exp>
                | <comp-exp>
<comp-exp>    ::= <primary-exp> ['(' <expression> {',' <expression>}* ')']
<primary-exp> ::= <atomic-exp> | '(' <expression> ')'
<atomic-exp>  ::= <identifer>
<identifer>   ::= 'C'<num>'_'<num> | 'P'<num>'_'<num> | 'S' | <variable>
<variable> ::= {'a' | ... | 'z'}{'A' | ... | 'Z' | 'a' | ... | 'z' 
                               | '0' | '1' | ... | '9' | '_'}*
<comment>  ::= {any character except newline}*
```
White space (spaces and tabs) can appear between any two tokens and should be ignored.