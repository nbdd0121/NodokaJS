#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"
#include "c/stdlib.h"
#include "c/stdbool.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/pass.h"

#define PUSH(x) do{\
        assert(stackTop<stackLimit);\
        *(stackTop++)=(x);\
    }while (0)

#define POP() ({\
        assert(stackTop>typeStack);\
        *(--stackTop);\
    })

#define PEEK() ({\
        assert(stackTop-typeStack>0);\
        *(stackTop-1);\
    })

static bool is(uint8_t x, uint8_t type) {
    return (x & type) == x;
}

static bool canBe(uint8_t x, uint8_t type) {
    return !!(x & type);
}

bool nodoka_convPass(nodoka_code_emitter *emitter) {
    uint8_t *typeStack = malloc(128);
    uint8_t *stackTop = typeStack;
    uint8_t *stackLimit = typeStack + 128;
    bool modified = false;
    for (size_t i = 0; i < emitter->bytecodeLength;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(emitter, &i);
        switch (bc) {
            case NODOKA_BC_LOAD_STR: {
                i += 2;
                PUSH(NODOKA_STRING);
                break;
            }
            case NODOKA_BC_LOAD_NUM: {
                i += 8;
                PUSH(NODOKA_NUMBER);
                break;
            }
            case NODOKA_BC_NOP: {
                break;
            }
            case NODOKA_BC_XCHG: {
                uint8_t sp0 = POP();
                uint8_t sp1 = POP();
                PUSH(sp0);
                PUSH(sp1);
                break;
            }
            case NODOKA_BC_RET: {
                goto ret;
            }
            case NODOKA_BC_NUM: {
                uint8_t type = POP();
                PUSH(NODOKA_NUMBER);
                if (is(type, NODOKA_NUMBER)) {
                    emitter->bytecode[i - 1] = NODOKA_BC_NOP;
                    modified = true;
                }
                break;
            }
            case NODOKA_BC_GET: {
                if (!canBe(PEEK(), NODOKA_REFERENCE)) {
                    emitter->bytecode[i - 1] = NODOKA_BC_NOP;
                    modified = true;
                } else {
                    assert(0);
                }
                break;
            }
            case NODOKA_BC_NEG:
            case NODOKA_BC_NOT:
            case NODOKA_BC_L_NOT: {
                break;
            }
            case NODOKA_BC_MUL:
            case NODOKA_BC_MOD:
            case NODOKA_BC_DIV:
            case NODOKA_BC_SUB: {
                POP();
                break;
            }
            case NODOKA_BC_ADD: {
                POP();
                POP();
                PUSH(NODOKA_STRING | NODOKA_NUMBER);
                break;
            }
            default: break; assert(0);
        }
    }
ret:
    free(typeStack);
    return modified;
}

