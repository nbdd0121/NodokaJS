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
     * [imm8] LOAD_STR
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




    NODOKA_BC_ADD,

};

struct nodoka_code {
    nodoka_data base;
    nodoka_string *stringPool;
    nodoka_code *codePool;
    uint8_t *bytecode;
    size_t strPoolLength;
    size_t codePoolLength;
    size_t bytecodeLength;
};

struct nodoka_code_emitter {
    nodoka_string *stringPool;
    nodoka_code *codePool;
    uint8_t *bytecode;
    size_t strPoolLength;
    size_t codePoolLength;
    size_t bytecodeLength;
    size_t strPoolCapacity;
    size_t codePoolCapacity;
    size_t bytecodeCapacity;
};