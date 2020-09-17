![build](https://github.com/PhilippHochmann/ccalc/workflows/build/badge.svg)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)

# ccalc
Scientific calculator in which you can define new functions and constants

## Setup guide
### Arch and Manjaro
You can install package ```ccalc``` directly from the AUR.

### Install from GitHub
1. Clone repository.
2. If you want to use readline, download its development files (Ubuntu: ```sudo apt-get install libreadline-dev```).
3. In root of repository, invoke ```make``` (optional targets: ```debug```, ```tests```, ```noreadline```).

## How to use it
When starting the calculator normally, you can enter expressions and commands interactively. Passed arguments will be evaluated beforehand. You can pipe in contents which will be evaluated as if typed in, after that the calculator terminates and does not enter interactive mode.

### Syntax
* Just type in a mathematical expression to evaluate it.
* Use ```ans``` or ```@<index>``` to reference previous results.
* Two subexpressions next to each other without an infix operator will be multiplied (e.g. ```2a``` or ```(x-1)(y+1)```).
* You can define functions and constants (e.g. ```myFunc(x) = x^2```, ```myConst = 42```).
* Any line starting with ```'``` will be ignored (useful for comments in files to be loaded).
* Use ```$``` to parse the rest of the expression as if it was put in parentheses, like in Haskell.

### Available commands
| Command                       | Description                                                               |
| ---                           | ---                                                                       |
| ```<func\|const> = <after>``` | Adds or redefines function or constant.                                   |
| ```table <expr> ; <from> ; <to> ; <step> [fold <expr> ; <init>]``` | Prints table of values and optionally folds them. In fold expression, ```x``` is replaced with the intermediate result (init in first step), ```y``` is replaced with the current value. Result of fold is stored in history. |
| ```load <path>```             | Loads file as if its content had been typed in.                           |
| ```help [operators]```        | Lists available commands and operators.                                   |
| ```clear [<func>]```          | Clears all or one function or constant.                                   |
| ```quit```                    | Closes application.                                                       |

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
| Name    | Precedence | Description                |
| ---     | ---        | ---                        |
| ```!``` | 5          | Factorial                  |
| ```%``` | 5          | Division by 100            |
| ```'``` | 7          | Derivation (experimental!) |

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
| ```sgn(x)```         | Sign of x (-1, 0, 1)                           |
| ```sum(*)```         | Sum of all operands                            |
| ```prod(*)```        | Product of all operands                        |
| ```avg(*)```         | Arithmetic mean of all operands                |
| ```gcd(x, y)```      | Greatest common divisor                        |
| ```lcm(x, y)```      | Least common multiple                          |
| ```rand(min, max)``` | Random integer between min and max (exclusive) |
| ```fib(n)```         | Fibonacci sequence                             |
| ```gamma(x)```       | Gamma function                                 |

Note:
* ```*``` is used to denote arbitrary number of operands
* Where operands are expected to be integer-valued, they will be truncated
* A function will return ```NaN``` on otherwise malformed arguments

### Constants
| Name         | Value         | Description                          |
| ---          | ---:          | ---                                  |
| ```pi```     | 3.14159265359 | Archimedes' constant                 |
| ```e```      | 2.71828182846 | Euler's number                       |
| ```phi```    | 1.61803398874 | Golden ratio                         |
| ```clight``` | 299792458     | Speed of light [m/s]                 |
| ```csound``` | 343.2         | Speed of sound in air at 20 °C [m/s] |
