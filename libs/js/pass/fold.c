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
        nodoka_data* ret;\
        if(stackTop<=typeStack){\
            ret=NULL;\
        }else{\
            ret=*(--stackTop);\
        }\
        ret;\
    })

#define PEEK() ({\
        nodoka_data* ret;\
        if(stackTop<=typeStack){\
            ret=NULL;\
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

bool nodoka_foldPass(nodoka_code_emitter *emitter, nodoka_code_emitter *target, size_t start, size_t end) {
    nodoka_data **typeStack = malloc(128);
    nodoka_data **stackTop = typeStack;
    nodoka_data **stackLimit = typeStack + 128;
    bool modified = false;
    for (size_t i = start; i < end;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(emitter, &i);
        switch (bc) {
            case NODOKA_BC_UNDEF: {
                PUSH(nodoka_undefined);
                break;
            }
            case NODOKA_BC_NULL: {
                PUSH(nodoka_null);
                break;
            }
            case NODOKA_BC_TRUE: {
                PUSH(nodoka_true);
                break;
            }
            case NODOKA_BC_FALSE: {
                PUSH(nodoka_false);
                break;
            }
            case NODOKA_BC_LOAD_STR: {
                nodoka_string *str = emitter->stringPool[nodoka_pass_fetch16(emitter, &i)];
                PUSH((nodoka_data *)str);
                nodoka_emitBytecode(target, bc, str);
                continue;
            }
            case NODOKA_BC_LOAD_NUM: {
                double val = int2double(nodoka_pass_fetch64(emitter, &i));
                PUSH((nodoka_data *)nodoka_newNumber(val));
                nodoka_emitBytecode(target, bc, val);
                continue;
            }
            case NODOKA_BC_NOP: {
                continue;
            }
            case NODOKA_BC_DUP: {
                nodoka_data *sp0 = POP();
                PUSH(sp0);
                PUSH(sp0);
                break;
            }
            case NODOKA_BC_POP: {
                POP();
                break;
            }
            case NODOKA_BC_XCHG: {
                nodoka_data *sp0 = POP();
                nodoka_data *sp1 = POP();
                PUSH(sp0);
                PUSH(sp1);
                break;
            }
            case NODOKA_BC_RET: {
                break;
            }
            case NODOKA_BC_L_NOT: {
                nodoka_data *sp0 = POP();
                if (sp0) {
                    assertBoolean(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, sp0 == nodoka_true ? NODOKA_BC_FALSE : NODOKA_BC_TRUE);
                    PUSH(sp0 == nodoka_true ? nodoka_false : nodoka_true);
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            case NODOKA_BC_ADD: {
                nodoka_data *sp0 = POP();
                nodoka_data *sp1 = POP();
                if (sp0 && sp1) {
                    assertPrimitive(sp1);
                    assertPrimitive(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    if (sp1->type == NODOKA_STRING || sp0->type == NODOKA_STRING) {
                        nodoka_string *lstr = nodoka_toString(sp1);
                        nodoka_string *rstr = nodoka_toString(sp0);
                        nodoka_string *result = nodoka_concatString(lstr, rstr);
                        PUSH((nodoka_data *)result);
                        nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, result);
                    } else {
                        nodoka_number *lnum = nodoka_toNumber(sp1);
                        nodoka_number *rnum = nodoka_toNumber(sp0);
                        double value = lnum->value + rnum->value;
                        PUSH((nodoka_data *)nodoka_newNumber(value));
                        nodoka_emitBytecode(target, NODOKA_BC_LOAD_NUM, value);
                    }
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            default: {
                assert(0);
                break;
            }
        }
        nodoka_emitBytecode(target, bc);
    }
ret:
    free(typeStack);
    return modified;
}

