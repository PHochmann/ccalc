![build](https://github.com/PhilippHochmann/ccalc/workflows/build/badge.svg)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)
[![AUR](https://img.shields.io/aur/version/ccalc)](https://aur.archlinux.org/packages/ccalc/)

# ccalc
Calculator with expression simplification that lets you define new functions and constants. Dependency-free except for readline.

## Setup guide

### Arch and Manjaro
You can install package `ccalc` directly from the AUR.

### Install from GitHub
1. Clone repository.
2. If you want to use readline, download its development files (Ubuntu: `sudo apt-get install libreadline-dev`).
3. In root of repository, invoke `make` (optional targets: `debug`, `tests`).
   If you don't want to use readline, add `NOREADLINE=1` as an argument.
4. If you automatically want to load simplification rules on startup, copy `simplification.ruleset` to `/etc/ccalc/`.
   If you want to use another folder, invoke `make INSTALL_PATH=/my/path` (without trailing slash) and place `simplification.ruleset` there.

## How to use it
`ccalc` processes input line by line. Any input that is not a command will be considered an arithmetical expression
and be directly evaluated. See syntax rules and commands below.

### Switches
| Switch                  | Description                                                          |
| ---                     | ---                                                                  |
| `--version -v`      | Displays version number and terminates.                              |
| `--help -h`         | Displays help message and terminates.                                |
| `--interactive -i`  | Forces to enter interactive mode after processing commands.          |
| `--quiet -q`        | Suppresses license notice on interactive start.                      |
| `--commands -c [N]` | Executes subsequent arguments as if typed in. Must be last switch. Terminates if `-i` not present. |

### Syntax
* Just type in a mathematical expression to evaluate it.
* Use `ans` or `@<index>` to reference previous results. `@0` is the same as `ans`, `@1` the second previous result and so on.
* Two subexpressions next to each other without an infix operator will be multiplied (e.g. `2a` or `(x-1)(y+1)`).
* You can define functions and constants (e.g. `myFunc(x) = x^2`, `myConst = 42`).
* Any line starting with `#` will be ignored (useful for comments in files to be loaded).
* Use `$` to parse the rest of the expression as if it was put in parentheses, like in Haskell.

### Available commands
| Command                            | Description                                                          |
| ---                                | ---                                                                  |
| `<func\|const> = <after>`      | Adds function or constant. E.g. `f(x)=3x^2` or `c=42`                |
| `table <expr> ; <from> ; <to> ; <step> [fold <expr> ; <init>]` | Prints table of values and optionally folds them. In fold expression, `x` is replaced with the intermediate result (init in first step), `y` is replaced with the current value. Result of fold is stored in history. |
| `load [simplification] <path>` | Loads file as if its content had been typed in or loads simplification rules. |
| `help [operators]`             | Lists available commands and operators.                              |
| `clear [<func\|const>]`         | Clears all or one function or constant.                              |
| `license`                      | Shows information about ccalc's license.                             |
| `quit`                         | Closes application.                                                  |

## Arithmetic operators

### Infix operators
| Name      | Associativity | Precedence | Description          |
| ---       | ---           | ---        | ---                  |
| `+`   | Both          | 2          | Addition             |
| `-`   | Left          | 2          | Subtraction          |
| `*`   | Both          | 4          | Multiplication       |
| `/`   | Left          | 3          | Division             |
| `^`   | Right         | 5          | Exponentiation       |
| `C`   | Left          | 1          | Binomial coefficient |
| `mod` | Left          | 1          | Modulo operator      |

### Prefix operators
| Name    | Precedence | Description |
| ---     | ---        | ---         |
| `$` | 0          | Identity to parse rest of expression as if put in parentheses |
| `@` | 8          | History operator (@0 = ans, @1 = second last result etc.) |
| `+` | 7          | Identity    |
| `-` | 7          | Negation    |

### Postfix operators
| Name    | Precedence | Description                 |
| ---     | ---        | ---                         |
| `!` | 6          | Factorial                   |
| `%` | 6          | Division by 100             |
| `'` | 7          | Derivative shorthand (beta) |

### Functions
| Name                 | Description                                    |
| ---                  | ---                                            |
| `deriv(x, y)`    | Derivative of x with respect to y (beta)       |
| `exp(x)`         | Natural exponential function                   |
| `root(x, n)`     | nth root of x                                  |
| `sqrt(x)`        | Square root                                    |
| `log(x, n)`      | Logarithm to base n                            |
| `ln(x)`          | Natural logarithm                              |
| `ld(x)`          | Binary logarithm                               |
| `lg(x)`          | Logarithm to base 10                           |
| `sin(x)`         | Sine                                           |
| `cos(x)`         | Cosine                                         |
| `tan(x)`         | Tangent                                        |
| `asin(x)`        | Inverse sine                                   |
| `acos(x)`        | Inverse cosine                                 |
| `atan(x)`        | Inverse tangens                                |
| `sinh(x)`        | Hyperbolic sine                                |
| `cosh(x)`        | Hyperbolic cosine                              |
| `tanh(x)`        | Hyperbolic tangent                             |
| `asinh(x)`       | Inverse hyperbolic sine                        |
| `acosh(x)`       | Inverse hyperbolic cosine                      |
| `atanh(x)`       | Inverse hyperbolic tangent                     |
| `max(*)`         | Maximum                                        |
| `min(*)`         | Minimum                                        |
| `abs(x)`         | Absolute value                                 |
| `ceil(x)`        | Round up to nearest integer                    |
| `floor(x)`       | Round down to nearest integer                  |
| `round(x)`       | Round to nearest integer                       |
| `trunc(x)`       | Round towards 0 to nearest integer             |
| `frac(x)`        | Fractional part of x                           |
| `sgn(x)`         | Sign of x (-1, 0, 1)                           |
| `sum(*)`         | Sum of all operands                            |
| `prod(*)`        | Product of all operands                        |
| `avg(*)`         | Arithmetic mean of all operands                |
| `median(*)`      | Median of all values                           |
| `gcd(x, y)`      | Greatest common divisor                        |
| `lcm(x, y)`      | Least common multiple                          |
| `rand(min, max)` | Random integer between min and max (exclusive) |
| `fib(n)`         | Fibonacci sequence                             |
| `gamma(x)`       | Gamma function                                 |
| `var(*)`         | Variance of a population                       |

Note:
* `*` is used to denote arbitrary number of operands
* Where operands are expected to be integer-valued, they will be truncated
* A function will return `NaN` on otherwise malformed arguments

### Constants
| Name         | Value         | Description                          |
| ---          | ---:          | ---                                  |
| `pi`     | 3.14159265359 | Archimedes' constant                 |
| `e`      | 2.71828182846 | Euler's number                       |
| `phi`    | 1.61803398874 | Golden ratio                         |
| `clight` | 299792458     | Speed of light [m/s]                 |
| `csound` | 343.2         | Speed of sound in air at 20 °C [m/s] |
| `ans`    |               | Last result                          |

## Contributing
Not currently accepting contributions. Feel free to create an issue.
