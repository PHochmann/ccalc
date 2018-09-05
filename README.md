# Calculator
Simple parser and calculator written in C

## Setup guide
1. Clone repository.

2. Download and install readline (On Ubuntu: ```sudo apt-get install libreadline-dev```)

3. In root directory, run ```chmod +x make.sh; ./make.sh``` to compile sources.

4. If you want, add alias such as ```c="~/Calculator/bin/calculator.out $@"``` in ~/.bash_aliases (when using the bash) to run program from anywhere. Arguments can be any expression as in interactive mode, which is started when program is executed without additional arguments.

## Available commands  
| Command | Description |
| --- | --- |
| debug | Display parsed abstract syntax tree after normal expression |
| help | List all available operators |
| def_func \<name\> \<arity\> | Add new function operator (arity of -1 is dynamic arity up to MAX_CHILDREN) |
| def_rule \<before\> -> \<after\> | Define new rule, e.g. to eliminate custom functions |

'ans' can be used to reference previous result.
