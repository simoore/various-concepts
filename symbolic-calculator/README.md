# Symbolic Calculator

This symbolic calculator allows the results of numerial operations to be 
saved into algebraic variables. Addition, subtraction, multiplication, and 
division are supported. Example usage of the calculator follows:

```
>x = 3*6 + 2
20.0
>y = (x - 8)/5
2.4
>y
2.4
```

Quit the application with an empty line.

## Installation

In the top level project directory, create a sandbox and build the application:

```
cabal sandbox init
cabal install
```

Execute the application with:

```
.cabal-sandbox\bin\sym-calc
```

## References

[Basics of Haskell](https://www.schoolofhaskell.com/school/starting-with-haskell/basics-of-haskell)

[How to Write a Haskell Program](https://wiki.haskell.org/How_to_write_a_Haskell_program)