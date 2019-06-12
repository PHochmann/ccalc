#define F_RED     "\x1B[1;31m"
#define F_GREEN   "\x1B[1;32m"
#define F_YELLOW  "\x1B[1;33m"
#define F_BLUE    "\x1B[34m"
#define F_MAGENTA "\x1B[35m"
#define F_CYAN    "\x1B[1;36m"
#define F_WHITE   "\x1B[37m"
#define F_BLACK   "\x1B[22;30m"
#define B_RED     "\x1B[1;41m"
#define B_GREEN   "\x1B[42m"
#define B_YELLOW  "\x1B[43m"
#define B_BLUE    "\x1B[44m"
#define B_MAGENTA "\x1B[45m"
#define B_CYAN    "\x1B[46m"
#define B_WHITE   "\x1B[47m"
#define B_BLACK   "\x1B[40m"
#define COL_RESET "\033[0m"

#define OP_COLOR    B_WHITE F_BLACK
#define CONST_COLOR F_YELLOW
#define VAR_COLOR   F_CYAN

// Maximum number of tokens reserved before tokenization
#define MAX_TOKENS 128
// Maximum number of operator/node instances the parser can handle at once
#define MAX_STACK_SIZE 128
// Used in matching and variable-list
#define MAX_VAR_COUNT 16
// Used to indicate arbitrary number of operands (0 up to 254)
#define DYNAMIC_ARITY 255
// Since arity is unsigned char and DYNAMIC_ARITY is set to 255
#define MAX_ARITY 254
// Associativity of OP_ASSOC_BOTH operands
#define STANDARD_ASSOC OP_ASSOC_LEFT
