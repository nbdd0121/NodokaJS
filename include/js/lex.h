#ifndef JS_LEX_H
#define JS_LEX_H

#include "unicode/convert.h"

#include "js/bytecode.h"

#define ASSIGN_FLAG 0x80
#define DOUBLE_FLAG 0x100
#define OTHER_FLAG 0x200
#define KEYWORD_FLAG 0x400

enum nodoka_nodoka_lexype {
    NODOKA_LEX_TOKEN,
    NODOKA_LEX_EMPTY_NODE,
    NODOKA_LEX_UNARY_NODE,
    NODOKA_LEX_BINARY_NODE,
    NODOKA_LEX_TERNARY_NODE
};

enum nodoka_token_type {
    NODOKA_TOKEN_LBRACE = '{',
    NODOKA_TOKEN_RBRACE = '}',
    NODOKA_TOKEN_LPAREN = '(',
    NODOKA_TOKEN_RPAREN = ')',
    NODOKA_TOKEN_LBRACKET = '[',
    NODOKA_TOKEN_RBRACKET = ']',
    NODOKA_TOKEN_POINT = '.',
    NODOKA_TOKEN_SEMICOLON = ';',
    NODOKA_TOKEN_COMMA = ',',
    NODOKA_TOKEN_NOT = '~',
    NODOKA_TOKEN_QUESTION = '?',
    NODOKA_TOKEN_COLON = ':',

    NODOKA_TOKEN_LT = '<',
    NODOKA_TOKEN_GT = '>',

    NODOKA_TOKEN_MUL = '*',
    NODOKA_TOKEN_DIV = '/',
    NODOKA_TOKEN_MOD = '%',
    NODOKA_TOKEN_XOR = '^',
    NODOKA_TOKEN_ADD = '+',
    NODOKA_TOKEN_SUB = '-',
    NODOKA_TOKEN_AND = '&',
    NODOKA_TOKEN_OR = '|',
    NODOKA_TOKEN_ASSIGN = '=',
    NODOKA_TOKEN_L_NOT = '!',


    NODOKA_TOKEN_MUL_ASSIGN = NODOKA_TOKEN_MUL | ASSIGN_FLAG,
    NODOKA_TOKEN_DIV_ASSIGN = NODOKA_TOKEN_DIV | ASSIGN_FLAG,
    NODOKA_TOKEN_MOD_ASSIGN = NODOKA_TOKEN_MOD | ASSIGN_FLAG,
    NODOKA_TOKEN_XOR_ASSIGN = NODOKA_TOKEN_XOR | ASSIGN_FLAG,
    NODOKA_TOKEN_ADD_ASSIGN = NODOKA_TOKEN_ADD | ASSIGN_FLAG,
    NODOKA_TOKEN_SUB_ASSIGN = NODOKA_TOKEN_SUB | ASSIGN_FLAG,
    NODOKA_TOKEN_AND_ASSIGN = NODOKA_TOKEN_AND | ASSIGN_FLAG,
    NODOKA_TOKEN_OR_ASSIGN = NODOKA_TOKEN_OR | ASSIGN_FLAG,
    NODOKA_TOKEN_EQ = NODOKA_TOKEN_ASSIGN | ASSIGN_FLAG,
    NODOKA_TOKEN_INEQ = NODOKA_TOKEN_L_NOT | ASSIGN_FLAG,

    NODOKA_TOKEN_LTEQ = NODOKA_TOKEN_LT | ASSIGN_FLAG,
    NODOKA_TOKEN_GTEQ = NODOKA_TOKEN_GT | ASSIGN_FLAG,


    NODOKA_TOKEN_INC = NODOKA_TOKEN_ADD | DOUBLE_FLAG,
    NODOKA_TOKEN_DEC = NODOKA_TOKEN_SUB | DOUBLE_FLAG,
    NODOKA_TOKEN_L_AND = NODOKA_TOKEN_AND | DOUBLE_FLAG,
    NODOKA_TOKEN_L_OR = NODOKA_TOKEN_OR | DOUBLE_FLAG,

    NODOKA_TOKEN_STRICT_EQ = OTHER_FLAG,
    NODOKA_TOKEN_STRICT_INEQ,
    NODOKA_TOKEN_STR,
    NODOKA_TOKEN_NUM,
    NODOKA_TOKEN_ID,
    NODOKA_TOKEN_REGEXP,

    NODOKA_TOKEN_SHL,
    NODOKA_TOKEN_SHR,
    NODOKA_TOKEN_USHR,
    NODOKA_TOKEN_SHL_ASSIGN,
    NODOKA_TOKEN_SHR_ASSIGN,
    NODOKA_TOKEN_USHR_ASSIGN,

    NODOKA_TOKEN_BREAK = KEYWORD_FLAG,
    NODOKA_TOKEN_CASE,
    NODOKA_TOKEN_CATCH,
    NODOKA_TOKEN_CONTINUE,
    NODOKA_TOKEN_DEBUGGER,
    NODOKA_TOKEN_DEFAULT,
    NODOKA_TOKEN_DELETE,
    NODOKA_TOKEN_DO,
    NODOKA_TOKEN_ELSE,
    NODOKA_TOKEN_FINALLY,
    NODOKA_TOKEN_FOR,
    NODOKA_TOKEN_FUNCTION,
    NODOKA_TOKEN_IF,
    NODOKA_TOKEN_IN,
    NODOKA_TOKEN_INSTANCEOF,
    NODOKA_TOKEN_NEW,
    NODOKA_TOKEN_RETURN,
    NODOKA_TOKEN_SWITCH,
    NODOKA_TOKEN_THIS,
    NODOKA_TOKEN_THROW,
    NODOKA_TOKEN_TRY,
    NODOKA_TOKEN_TYPEOF,
    NODOKA_TOKEN_VAR,
    NODOKA_TOKEN_VOID,
    NODOKA_TOKEN_WHILE,
    NODOKA_TOKEN_WITH,

    NODOKA_TOKEN_EOF,

    NODOKA_TOKEN_NULL,
    NODOKA_TOKEN_TRUE,
    NODOKA_TOKEN_FALSE,

    NODOKA_TOKEN_RESERVED_WORD,
    NODOKA_TOKEN_RESERVED_STRICT,
};

enum nodoka_empty_node_type {
    EMPTY_STMT,
    DEBUGGER_STMT,
};

enum nodoka_unary_node_type {
    POST_INC_NODE,//TODO
    POST_DEC_NODE,//TODO
    DELETE_NODE,//TODO

    NODOKA_VOID_NODE,

    TYPEOF_NODE,//TODO
    PRE_INC_NODE,//TODO
    PRE_DEC_NODE,//TODO

    NODOKA_POS_NODE,
    NODOKA_NEG_NODE,
    NODOKA_NOT_NODE,
    NODOKA_LNOT_NODE,

    EXPR_STMT,
};

enum nodoka_binary_node_type {
    NODOKA_MEMBER_NODE,//TODO

    NODOKA_MUL_NODE,
    NODOKA_MOD_NODE,
    NODOKA_DIV_NODE,
    NODOKA_ADD_NODE,
    NODOKA_SUB_NODE,
    NODOKA_SHL_NODE,
    NODOKA_SHR_NODE,
    NODOKA_USHR_NODE,
    NODOKA_LT_NODE,
    NODOKA_GT_NODE,
    NODOKA_LTEQ_NODE,
    NODOKA_GTEQ_NODE,

    NODOKA_INSTANCEOF_NODE,//TODO
    NODOKA_IN_NODE,//TODO

    NODOKA_EQ_NODE,
    NODOKA_INEQ_NODE,
    NODOKA_STRICT_EQ_NODE,
    NODOKA_STRICT_INEQ_NODE,
    NODOKA_AND_NODE,
    NODOKA_XOR_NODE,
    NODOKA_OR_NODE,
    NODOKA_L_AND_NODE,
    NODOKA_L_OR_NODE,//TODO

    NODOKA_ASSIGN_NODE,//TODO
    NODOKA_MUL_ASSIGN_NODE,//TODO
    NODOKA_DIV_ASSIGN_NODE,//TODO
    NODOKA_MOD_ASSIGN_NODE,//TODO
    NODOKA_ADD_ASSIGN_NODE,//TODO
    NODOKA_SUB_ASSIGN_NODE,//TODO
    NODOKA_SHL_ASSIGN_NODE,//TODO
    NODOKA_SHR_ASSIGN_NODE,//TODO
    NODOKA_USHR_ASSIGN_NODE,//TODO
    NODOKA_AND_ASSIGN_NODE,//TODO
    NODOKA_XOR_ASSIGN_NODE,//TODO
    NODOKA_OR_ASSIGN_NODE,//TODO

    NODOKA_COMMA_NODE,

    BLOCK_STMT_LIST,
};

enum nodoka_ternary_node_type {
    COND_NODE,//TODO
    IF_STMT,
};

typedef struct nodoka_lex_class {
    enum nodoka_nodoka_lexype clazz;
} nodoka_lex_class;

typedef struct nodoka_token {
    nodoka_lex_class base;
    enum nodoka_token_type type;
    struct nodoka_token *next;
    union {
        utf16_string_t stringValue;
        double numberValue;
    };
    bool lineBefore;
} nodoka_token;

typedef struct nodoka_empty_node {
    nodoka_lex_class base;
    enum nodoka_empty_node_type type;
} nodoka_empty_node;

typedef struct nodoka_unary_node {
    nodoka_lex_class base;
    enum nodoka_unary_node_type type;
    nodoka_lex_class *_1;
} nodoka_unary_node;

typedef struct nodoka_binary_node {
    nodoka_lex_class base;
    enum nodoka_binary_node_type type;
    nodoka_lex_class *_1;
    nodoka_lex_class *_2;
} nodoka_binary_node;

typedef struct nodoka_ternary_node {
    nodoka_lex_class base;
    enum nodoka_ternary_node_type type;
    nodoka_lex_class *_1;
    nodoka_lex_class *_2;
    nodoka_lex_class *_3;
} nodoka_ternary_node;

typedef struct struct_lex nodoka_lex;

struct struct_lex {
    uint16_t (*next)(nodoka_lex *lex);
    uint16_t (*lookahead)(nodoka_lex *lex);
    nodoka_token *(*state)(nodoka_lex *lex);
    utf16_string_t content;
    size_t ptr;
    bool regexp;
    bool strictMode;
    bool lineBefore;
    bool parseId;
    union {
        struct {
            uint16_t *buffer;
            size_t size;
            size_t length;
        };
        double number;
    } data;
};

typedef struct struct_grammar nodoka_grammar;

nodoka_lex *lex_new(char *chr);
nodoka_token *lex_next(nodoka_lex *lex);
nodoka_grammar *grammar_new(nodoka_lex *lex);
void nodoka_codegen(nodoka_code_emitter *emitter, nodoka_lex_class *node);
void nodoka_disposeLexNode(nodoka_lex_class *node);

#endif

