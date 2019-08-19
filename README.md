# Calculator
Simple calculator written in C in which you can define your own functions and pattern matching rules

## Setup guide
1. Clone repository.

2. Download and install readline. (On Ubuntu: ```sudo apt-get install libreadline-dev```)

3. In root directory, run ```./make.sh [-d]``` to compile sources.

4. If you want, add alias such as ```c="~/Calculator/bin/calculator.out $@"``` in ~/.bash_aliases (when using the bash) to run program from anywhere. Arguments can be any expression as in interactive mode, which is started when program is executed without additional arguments.

## How to use it
When starting the calculator normally, you can enter expressions and commands interactively. Passed arguments will be evaluated beforehand. You can pipe in contents which will be evaluated as if typed in, after that the calculator terminates and does not enter interactive mode.

### Syntax
* Any input which is not a command will be interpreted as an expression.
* ```ans``` can be used to reference previous result.
* Two subexpressions next to each other without an infix operator will be multiplied (e.g. ```2a``` or ```(x-1)(y+1)```).
* To define a constant, you can define a function without arguments (e.g. ```myConst() := 42```) or a pattern matching rule that only applies to a variable of a specific name (e.g. ```name_myConst -> 42```).
* When applying unary functions to a literal, you can omit parentheses (e.g. ```sin2```).
* Any line starting with ```'``` will be ignored (useful for comments in files to be loaded).

### Available commands
| Command                     | Description                                                                           |
| ---                         | ---                                                                                   |
| ```debug```                 | Toggles debug mode. In debug mode, an abstract syntax tree is shown before evaluation |
| ```help```                  | Lists all available operators                                                         |
| ```rules [clear]```         | Lists all defined rules or clears all rules and defined functions                     |
| ```<function> := <after>``` | Adds new function                                                                     |
| ```<before> -> <after>```   | Defines new rule. The following special variable names can be used to restrict matchings (<tt>x</tt> used as example, can be any variable name): <ul><li><tt>name_x</tt> will only bind to variables named x</li><li><tt>var_x</tt> will only bind to variables</li><li><tt>const_x</tt> will only bind to constants</li></ul> |
| ```load <path>```           | Loads file as if its content had been typed in line by line                           |
| ```quit```                  | Closes calculator                                                                     |

### Infix operators
| Name      | Associativity | Precedence | Description          |
| ---       | ---           | ---        | ---                  |
| ```+```   | Both          | 2          | Addition             |
| ```-```   | Left          | 2          | Subtraction          |
| ```*```   | Both          | 3          | Multiplication       |
| ```/```   | Left          | 3          | Division             |
| ```^```   | Right         | 4          | Exponentiation       |
| ```C```   | Left          | 1          | Binomial coefficient |
| ```mod``` | Left          | 1          | Modulo operator      |

### Prefix operators
| Name    | Precedence | Description |
| ---     | ---        | ---         |
| ```+``` | 6          | Identity    |
| ```-``` | 6          | Negation    |

### Postfix operators
| Name    | Precedence | Description     |
| ---     | ---        | ---             |
| ```!``` | 5          | Factorial       |
| ```%``` | 5          | Division by 100 |

### Functions
| Name                 | Description                                    |
| ---                  | ---                                            |
| ```exp(x)```         | Natural exponential function                   |
| ```root(x, n)```     | nth root of x                                  |
| ```sqrt(x)```        | Square root                                    |
| ```log(x, n)```      | Logarithm to base n                            |
| ```ln(x)```          | Natural logarithm                              |
| ```ld(x)```          | Binary logarithm                               |
| ```lg(x)```          | Logarithm to base 10                           |
| ```sin(x)```         | Sine                                           |
| ```cos(x)```         | Cosine                                         |
| ```tan(x)```         | Tangent                                        |
| ```asin(x)```        | Inverse sine                                   |
| ```acos(x)```        | Inverse cosine                                 |
| ```atan(x)```        | Inverse tangens                                |
| ```sinh(x)```        | Hyperbolic sine                                |
| ```cosh(x)```        | Hyperbolic cosine                              |
| ```tanh(x)```        | Hyperbolic tangent                             |
| ```asinh(x)```       | Inverse hyperbolic sine                        |
| ```acosh(x)```       | Inverse hyperbolic cosine                      |
| ```atanh(x)```       | Inverse hyperbolic tangent                     |
| ```max(*)```         | Maximum                                        |
| ```min(*)```         | Minimum                                        |
| ```abs(x)```         | Absolute value                                 |
| ```ceil(x)```        | Round up to nearest integer                    |
| ```floor(x)```       | Round down to nearest integer                  |
| ```round(x)```       | Round to nearest integer                       |
| ```trunc(x)```       | Round towards 0 to nearest integer             |
| ```frac(x)```        | Fractional part of x                           |
| ```sum(*)```         | Sum of all operands                            |
| ```prod(*)```        | Product of all operands                        |
| ```avg(*)```         | Arithmetic mean of all operands                |
| ```rand(min, max)``` | Random integer between min and max (exclusive) |
| ```gamma(x)```       | Gamma function                                 |
| ```fib(n)```         | Fibonacci sequence                             |

Note:
* ```*``` is used to denote arbitrary number of operands
* Where operands are expected to be integer-valued, they will be truncated

### Constants
| Name         | Value         | Description                          |
| ---          | ---           | ---                                  |
| ```pi```     | 3.14159265359 | Archimedes' constant                 |
| ```e```      | 2.71828182846 | Euler's number                       |
| ```phi```    | 1.61803398874 | Golden ratio                         |
| ```clight``` | 299792458     | Speed of light [m/s]                 |
| ```csound``` | 343.2         | Speed of sound in air at 20 °C [m/s] |
