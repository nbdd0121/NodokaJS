#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"
#include "c/stdlib.h"
#include "c/stdbool.h"

#include "util/double.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/pass.h"

#define PUSH(x) do{\
        assert(stackTop<stackLimit);\
        *(stackTop++)=(x);\
    }while (0)

#define POP() ({\
        uint8_t ret;\
        if(stackTop<=typeStack){\
            ret=0xFF;\
        }else{\
            ret=*(--stackTop);\
        }\
        ret;\
    })

#define PEEK() ({\
        uint8_t ret;\
        if(stackTop<=typeStack){\
            ret=0xFF;\
        }else{\
            ret=*(stackTop-1);\
        }\
        ret;\
    })

static bool isPrim(uint8_t x) {
    return (x & (NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING)) == x;
}

static bool canBeRef(uint8_t x) {
    return !!(x & NODOKA_REFERENCE);
}

bool nodoka_convPass(nodoka_code_emitter *emitter, nodoka_code_emitter *target, size_t start, size_t end) {
    uint8_t *typeStack = malloc(128);
    uint8_t *stackTop = typeStack;
    uint8_t *stackLimit = typeStack + 128;
    bool modified = false;
    for (size_t i = start; i < end;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(emitter, &i);
        switch (bc) {
            case NODOKA_BC_UNDEF: {
                PUSH(NODOKA_UNDEF);
                goto emitToTarget;
            }
            case NODOKA_BC_NULL: {
                PUSH(NODOKA_NULL);
                goto emitToTarget;
            }
            case NODOKA_BC_TRUE: {
                PUSH(NODOKA_BOOL);
                goto emitToTarget;
            }
            case NODOKA_BC_FALSE: {
                PUSH(NODOKA_BOOL);
                goto emitToTarget;
            }
            case NODOKA_BC_LOAD_STR: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                PUSH(NODOKA_STRING);
                nodoka_emitBytecode(target, bc, emitter->stringPool[offset]);
                break;
            }
            case NODOKA_BC_LOAD_NUM: {
                double val = int2double(nodoka_pass_fetch64(emitter, &i));
                PUSH(NODOKA_NUMBER);
                nodoka_emitBytecode(target, bc, val);
                break;
            }
            case NODOKA_BC_NOP: {
                break;
            }
            case NODOKA_BC_DUP: {
                uint8_t sp0 = POP();
                PUSH(sp0);
                PUSH(sp0);
                goto emitToTarget;
            }
            case NODOKA_BC_POP: {
                POP();
                goto emitToTarget;
            }
            case NODOKA_BC_XCHG: {
                uint8_t sp0 = POP();
                uint8_t sp1 = POP();
                PUSH(sp0);
                PUSH(sp1);
                goto emitToTarget;
            }
            case NODOKA_BC_RET: {
                goto emitToTarget;
            }
            case NODOKA_BC_PRIM: {
                uint8_t type = POP();
                if (isPrim(type)) {
                    PUSH(type);
                    emitter->bytecode[i - 1] = NODOKA_BC_NOP;
                    modified = true;
                    break;
                } else {
                    PUSH(NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING);
                    goto emitToTarget;
                }
            }
            case NODOKA_BC_BOOL: {
                uint8_t type = POP();
                PUSH(NODOKA_BOOL);
                if (type == NODOKA_BOOL) {
                    emitter->bytecode[i - 1] = NODOKA_BC_NOP;
                    modified = true;
                    break;
                } else {
                    goto emitToTarget;
                }
            }
            case NODOKA_BC_NUM: {
                uint8_t type = POP();
                PUSH(NODOKA_NUMBER);
                if (type == NODOKA_NUMBER) {
                    emitter->bytecode[i - 1] = NODOKA_BC_NOP;
                    modified = true;
                    break;
                } else {
                    goto emitToTarget;
                }
            }
            case NODOKA_BC_GET: {
                if (!canBeRef(PEEK())) {
                    emitter->bytecode[i - 1] = NODOKA_BC_NOP;
                    modified = true;
                    break;
                } else {
                    assert(0);
                    goto emitToTarget;
                }
            }
            case NODOKA_BC_NEG:
            case NODOKA_BC_NOT:
            case NODOKA_BC_L_NOT: {
                goto emitToTarget;
            }
            case NODOKA_BC_MUL:
            case NODOKA_BC_MOD:
            case NODOKA_BC_DIV:
            case NODOKA_BC_SUB:
            case NODOKA_BC_SHL:
            case NODOKA_BC_SHR:
            case NODOKA_BC_USHR:
            case NODOKA_BC_AND:
            case NODOKA_BC_OR:
            case NODOKA_BC_XOR: {
                POP();
                goto emitToTarget;
            }
            case NODOKA_BC_ADD: {
                POP();
                POP();
                PUSH(NODOKA_STRING | NODOKA_NUMBER);
                goto emitToTarget;
            }
            case NODOKA_BC_LT:
            case NODOKA_BC_LTEQ:
            case NODOKA_BC_EQ:
            case NODOKA_BC_S_EQ: {
                POP();
                POP();
                PUSH(NODOKA_BOOL);
                goto emitToTarget;
            }
            default: {
                assert(0);
emitToTarget:
                nodoka_emitBytecode(target, bc);
                break;
            }
        }
    }
ret:
    free(typeStack);
    return modified;
}

