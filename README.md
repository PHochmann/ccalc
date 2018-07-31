# Calculator
Simple parser and calculator written in C

## Setup guide
1. Clone repository

2. In root directory, run

    ```chmod +x make.sh; ./make.sh```
    
to compile sources

3. Add alias such as

    ```c="~/Calculator/bin/calculator.out $@"```
    
in ~/.bash_aliases (when using the bash)
to run program from anywhere. Arguments can be any expression as in interactive mode,
which is started when program is executed without additional arguments)
  
Type 'debug' to visually see parsed abstract syntax tree, or 'help' to list available operators.
