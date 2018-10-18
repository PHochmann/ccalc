// Colours
#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"
#define F_YELLOW  "\x1B[1;33m"
#define F_BLUE    "\x1B[34m"
#define F_MAGENTA "\x1B[35m"
#define F_CYAN    "\x1B[1;36m"
#define F_WHITE   "\x1B[37m"
#define B_RED     "\x1B[1;41m"
#define B_GREEN   "\x1B[42m"
#define B_YELLOW  "\x1B[43m"
#define B_BLUE    "\x1B[44m"
#define B_MAGENTA "\x1B[45m"
#define B_CYAN    "\x1B[46m"
#define B_WHITE   "\x1B[47m"
#define B_BLACK   "\x1B[40m"

#define OP_COLOR B_RED F_GREEN
#define CONST_COLOR F_YELLOW
#define VAR_COLOR F_CYAN
#define COL_RESET F_WHITE B_BLACK

// Maximum number of tokens reserved before tokenization
#define MAX_TOKENS 128

// Maximum number of operator/node instances the parser can handle at once
#define MAX_STACK_SIZE 128

// Maximum length of name of an operator
#define MAX_OP_LENGTH 8

// Maximum length of a variable's name
#define MAX_VAR_LENGTH 8

// Maximum arity of an operator (Max. number of children a node can have)
#define MAX_CHILDREN 16

// Used to indicate arbitrary amount of operands (0 up to MAX_CHILDREN)
#define DYNAMIC_ARITY -1

// Used in matching and variable-list
#define MAX_VAR_COUNT 16
