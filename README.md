# libabacus
A math-centered programming language library.

## About
libabacus is effectively a stripped down, embeddable programming language.
It relies on the program it's embedded into to provide the standard library. In fact,
it doesn't even provide an underlying implementation for numbers - the client code
provides a means of convering a string into a number, and a way of freeing that value.
libabacus takes care of the rest. The features of libabacus are geared towards 
calculators, so it does not provide higher-level abstractions like OOP.

## Syntax
libabacus has fairly simple syntax. A simple number expression is
```
42
```
If an operator `+` is registered with libabacus, then an expression using
that operator looks like
```
21+21
```
If two functions, `f` and `g` are registered with libabacus, where `f` takes
one argument and `g` takes two, then the syntax for calling these functions is
```
f(42)
g(21, 21)
```
Blocks are a way to combine a sequence of expressions into one. The value of a block
is the result of the last expression in that block. Expressions are delimited by the
`;` character. For example,
```
{ 3; 2; 1 }
```
evalutes to `1`. Blocks are the first bit of syntax we've seen that can evalute to
a value that isn't a number. For instance, the empty block, `{}`, does not
evaluate to a number. Instead, it evalutes to `()`. This is a value of the `unit` type,
which only has one possible value, `()`.

User-defined functions are supported by libabacus:
```
fun square(a: num): num { a*a }
```
Let's pick this apart. The first part of a function declaration is the `fun` keyword. The name
of the function is next, followed by the list of parameters that this function accepts. Parameters
are listed in the form `[name]:[type]`. The return type of the function must also be specified.
Lastly, the block used to evaluate the function is needed. 

The block captures the scope in which the function was declared. For instance, in the following piece of code,
```
a = 0;
fun test(): num { 
    a = a + 1
}
```
`test` actually has access to `a`, and calling `test` many times will yield increasing values of a.

Although all parameters must specify a type, this type can be
arbitrary to allow for polymorphic functions. For instance, a function to apply another function to an argument
twice can be written as:
```
fun apply_twice(func: ('T)->'T, a: 'T): 'T {
    func(a);
    func(a)
}
```

## Integration
Everything in libabacus revolves around the `libab` struct. This struct is used to keep all the state related to the evaluation,
and also contains the garbage collector. It needs to be instantiated using the `libab_init` function.
```C
libab ab;
libab_init(&ab, parse_function, free_function);
```
Please see `interactive.c` for an example of a simple yet complete implementation.
