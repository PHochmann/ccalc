// Colours
#define F_RED     "\x1B[31m"
#define F_GREEN   "\x1B[32m"
#define F_YELLOW  "\x1B[33m"
#define F_BLUE    "\x1B[34m"
#define F_MAGENTA "\x1B[35m"
#define F_CYAN    "\x1B[36m"
#define F_WHITE   "\x1B[37m"
#define B_RED     "\x1B[41m"
#define B_GREEN   "\x1B[42m"
#define B_YELLOW  "\x1B[43m"
#define B_BLUE    "\x1B[44m"
#define B_MAGENTA "\x1B[45m"
#define B_CYAN    "\x1B[46m"
#define B_WHITE   "\x1B[47m"
#define B_BLACK   "\x1B[40m"
#define COL_RESET "\x1B[0m" B_BLACK
#define OP_COLOR B_GREEN F_BLUE
#define CONST_COLOR F_YELLOW
#define VAR_COLOR F_CYAN
// - - -

// Maximum number of tokens reserved before tokenization
#define MAX_TOKENS 256

// Maximum number of operator/node instances the parser can handle at once
#define MAX_STACK_SIZE 256

// Maximum length of name of an operator
#define MAX_OP_LENGTH 8

// Maximum length of a variable's name
#define MAX_VAR_LENGTH 16

// Maximum arity of an operator (Max. number of children a node can have)
#define MAX_CHILDREN 16

// Used to indicate arbitrary amount of operands (0 up to MAX_CHILDREN)
#define DYNAMIC_ARITY -1