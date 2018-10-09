# Calculator
Simple parser and calculator written in C

## Setup guide
1. Clone repository.

2. Download and install readline. (On Ubuntu: ```sudo apt-get install libreadline-dev```)

3. In root directory, run ```chmod +x make.sh; ./make.sh``` to compile sources.

4. If you want, add alias such as ```c="~/Calculator/bin/calculator.out $@"``` in ~/.bash_aliases (when using the bash) to run program from anywhere. Arguments can be any expression as in interactive mode, which is started when program is executed without additional arguments.

## Available commands  
| Command | Description |
| --- | --- |
| ```debug``` | Toggles debug mode. In debug mode, an abstract syntax tree is shown before evaluation |
| ```help``` | Lists all available operators |
| ```<function> := <after>``` | Adds new function |
| ```<before> -> <after>``` | Defines new rule <ul><li><tt>literal_x</tt> will only bind to variables named  x (can be used with any other name)</li><li><tt>var_x</tt> will only bind to variables</li><li><tt>const_x</tt> will only bind to constants</li></ul> |

```ans``` can be used to reference previous result.
