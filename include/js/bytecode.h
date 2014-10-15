#ifndef JS_BYTECODE_H
#define JS_BYTECODE_H

#include "js/js.h"

enum nodoka_bytecode {
    /**
     * [] UNDEF
     * push undefined to stack
     */
    NODOKA_BC_UNDEF,

    /**
     * [] NULL
     * push null to stack
     */
    NODOKA_BC_NULL,

    /**
     * [] TRUE
     * push true to stack
     */
    NODOKA_BC_TRUE,

    /**
     * [] FALSE
     * push true to stack
     */
    NODOKA_BC_FALSE,

    /**
     * [imm16] LOAD_STR
     * push the corresponding constant string to stack
     */
    NODOKA_BC_LOAD_STR,

    /**
     * [imm64] LOAD_NUM
     * push the immediate number to stack
     */
    NODOKA_BC_LOAD_NUM,

    /**
     * [] NOP
     * do nothing
     */
    NODOKA_BC_NOP,

    /**
     * [] DUP
     * duplicate the top element
     */
    NODOKA_BC_DUP,

    /**
     * [] POP
     * pop the top element from the stack
     */
    NODOKA_BC_POP,

    /**
     * [] XCHG
     * exchange the top 2 elements
     */
    NODOKA_BC_XCHG,

    NODOKA_BC_XCHG3,

    /**
     * [] RET
     * return the top element
     */
    NODOKA_BC_RET,

    NODOKA_BC_THIS,

    /**
     * [] PRIM
     * convert the stack top to primitive
     */
    NODOKA_BC_PRIM,

    /**
     * [] BOOL
     * convert the stack top to boolean
     */
    NODOKA_BC_BOOL,

    /**
     * [] NUM
     * convert the stack top to number
     */
    NODOKA_BC_NUM,

    /**
     * [] STR
     * convert the stack top to string
     */
    NODOKA_BC_STR,

    NODOKA_BC_REF,

    /**
     * [] GET
     * convert the top reference (if it is) to its value
     */
    NODOKA_BC_GET,

    NODOKA_BC_PUT,

    NODOKA_BC_DEL,

    NODOKA_BC_CALL,


    /**
     * [] NEG
     * negate sp0. NaN will still be NaN
     * @Precondition sp0 is number
     */
    NODOKA_BC_NEG,
    NODOKA_BC_NOT,
    NODOKA_BC_L_NOT,
    NODOKA_BC_MUL,
    NODOKA_BC_MOD,
    NODOKA_BC_DIV,
    NODOKA_BC_ADD,
    NODOKA_BC_SUB,
    NODOKA_BC_SHL,
    NODOKA_BC_SHR,
    NODOKA_BC_USHR,
    NODOKA_BC_LT,
    NODOKA_BC_LTEQ,
    NODOKA_BC_EQ,
    NODOKA_BC_S_EQ,
    NODOKA_BC_AND,
    NODOKA_BC_OR,
    NODOKA_BC_XOR,

    NODOKA_BC_JMP,
    NODOKA_BC_JT,

    NODOKA_BC_THROW,


};

struct nodoka_code {
    nodoka_data base;
    nodoka_string **stringPool;
    nodoka_code **codePool;
    uint8_t *bytecode;
    size_t strPoolLength;
    size_t codePoolLength;
    size_t bytecodeLength;
};

struct nodoka_code_emitter {
    nodoka_string **stringPool;
    nodoka_code **codePool;
    uint8_t *bytecode;
    size_t strPoolLength;
    size_t codePoolLength;
    size_t bytecodeLength;
    size_t strPoolCapacity;
    size_t codePoolCapacity;
    size_t bytecodeCapacity;
};

typedef struct nodoka_envRec {
    struct nodoka_envRec *outer;
    nodoka_object *object;
    nodoka_data *this;
} nodoka_envRec;

struct nodoka_context {
    nodoka_envRec *env;
    nodoka_code *code;
    nodoka_data **stack;
    nodoka_data **stackTop;
    nodoka_data **stackLimit;
    nodoka_object *this;
    size_t insPtr;
};

typedef uint16_t nodoka_relocatable;
typedef uint16_t nodoka_label;

nodoka_code_emitter *nodoka_newCodeEmitter(void);
void nodoka_emitBytecode(nodoka_code_emitter *codeseg, uint8_t bc, ...);
nodoka_label nodoka_putLabel(nodoka_code_emitter *emitter);
void nodoka_relocate(nodoka_code_emitter *emitter, nodoka_relocatable rel, nodoka_label label);
void nodoka_xchgEmitter(nodoka_code_emitter *, nodoka_code_emitter *);
void nodoka_freeEmitter(nodoka_code_emitter *emitter);
nodoka_code *nodoka_packCode(nodoka_code_emitter *emitter);
nodoka_code_emitter *nodoka_unpackCode(nodoka_code *code);

nodoka_context *nodoka_newContext(nodoka_code *code, nodoka_envRec *env, nodoka_object *this);

nodoka_envRec *nodoka_newDeclEnvRecord(nodoka_envRec *outer);
nodoka_envRec *nodoka_newObjEnvRecord(nodoka_object *obj, nodoka_envRec *outer);

#endif
