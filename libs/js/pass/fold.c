#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"
#include "c/stdlib.h"
#include "c/stdbool.h"
#include "c/math.h"

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

bool nodoka_foldPass(nodoka_code_emitter *emitter, nodoka_code_emitter *target, size_t start, size_t end) {
    nodoka_data **typeStack = malloc(128);
    nodoka_data **stackTop = typeStack;
    nodoka_data **stackLimit = typeStack + 128;
    bool modified = false;
    for (size_t i = start; i < end;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(emitter, &i);
        switch (bc) {
            case NODOKA_BC_UNDEF: PUSH(nodoka_undefined); break;
            case NODOKA_BC_NULL: PUSH(nodoka_null); break;
            case NODOKA_BC_TRUE: PUSH(nodoka_true); break;
            case NODOKA_BC_FALSE: PUSH(nodoka_false); break;
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
            case NODOKA_BC_NOP: continue;
            case NODOKA_BC_DUP: {
                nodoka_data *sp0 = POP();
                PUSH(sp0);
                PUSH(sp0);
                break;
            }
            case NODOKA_BC_POP: POP(); break;
            case NODOKA_BC_XCHG: {
                nodoka_data *sp0 = POP();
                nodoka_data *sp1 = POP();
                PUSH(sp0);
                PUSH(sp1);
                break;
            }
            case NODOKA_BC_XCHG3: {
                nodoka_data *sp0 = POP();
                nodoka_data *sp1 = POP();
                nodoka_data *sp2 = POP();
                PUSH(sp0);
                PUSH(sp2);
                PUSH(sp1);
                break;
            }
            case NODOKA_BC_RET: break;
            case NODOKA_BC_THIS: PUSH(NULL); break;
            /* Notice that constants are all primitives, so this instruction needs no special deal */
            case NODOKA_BC_PRIM: break;
            case NODOKA_BC_BOOL: {
                nodoka_data *sp0 = POP();
                if (sp0) {
                    nodoka_data *result = nodoka_toBoolean(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, result == nodoka_true ? NODOKA_BC_TRUE : NODOKA_BC_FALSE);
                    PUSH(result);
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            case NODOKA_BC_NUM: {
                nodoka_data *sp0 = POP();
                if (sp0) {
                    nodoka_number *result = nodoka_toNumber(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_NUM, result->value);
                    PUSH((nodoka_data *)result);
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            case NODOKA_BC_STR: {
                nodoka_data *sp0 = POP();
                if (sp0) {
                    nodoka_string *result = nodoka_toString(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, result);
                    PUSH((nodoka_data *)result);
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            case NODOKA_BC_GET: break;
            case NODOKA_BC_PUT: POP(); POP(); break;
            case NODOKA_BC_REF: POP(); POP(); PUSH(NULL); break;
            case NODOKA_BC_DEL: {
                nodoka_data *sp0 = POP();
                if (sp0) {
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_TRUE);
                    PUSH(nodoka_true);
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            case NODOKA_BC_CALL: {
                uint8_t count = nodoka_pass_fetch8(emitter, &i);
                nodoka_emitBytecode(target, NODOKA_BC_CALL, count);
                for (int i = 0; i < count; i++) {
                    POP();
                }
                POP();
                PUSH(NULL);
                continue;
            }
            case NODOKA_BC_NEG: {
                nodoka_number *sp0 = (nodoka_number *)POP();
                if (sp0) {
                    assertNumber(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_NUM, -sp0->value);
                    PUSH((nodoka_data *)nodoka_newNumber(-sp0->value));
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
            }
            case NODOKA_BC_NOT: {
                nodoka_number *sp0 = (nodoka_number *)POP();
                if (sp0) {
                    assertNumber(sp0);
                    double value = ~nodoka_toInt32(sp0);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_NUM, value);
                    PUSH((nodoka_data *)nodoka_newNumber(value));
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
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
                nodoka_number *sp0 = (nodoka_number *)POP();
                nodoka_number *sp1 = (nodoka_number *)POP();
                if (sp0 && sp1) {
                    assertNumber(sp1);
                    assertNumber(sp0);
                    double value;
                    switch (bc) {
                        case NODOKA_BC_MUL: value = sp1->value * sp0->value; break;
                        case NODOKA_BC_MOD: value = fmod(sp1->value, sp0->value); break;
                        case NODOKA_BC_DIV: value = sp1->value / sp0->value; break;
                        case NODOKA_BC_SUB: value = sp1->value - sp0->value; break;
                        case NODOKA_BC_SHL: value = nodoka_toInt32(sp1) << (nodoka_toUint32(sp0) & 0x1F); break;
                        case NODOKA_BC_SHR: value = nodoka_toInt32(sp1) >> (nodoka_toUint32(sp0) & 0x1F); break;
                        case NODOKA_BC_USHR: value = nodoka_toUint32(sp1) >> (nodoka_toUint32(sp0) & 0x1F); break;
                        case NODOKA_BC_AND: value = nodoka_toInt32(sp1) & nodoka_toInt32(sp0); break;
                        case NODOKA_BC_OR: value = nodoka_toInt32(sp1) | nodoka_toInt32(sp0); break;
                        case NODOKA_BC_XOR: value = nodoka_toInt32(sp1) ^ nodoka_toInt32(sp0); break;
                        default: assert(0);
                    }
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_NUM, value);
                    PUSH((nodoka_data *)nodoka_newNumber(value));
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
            case NODOKA_BC_LT:
            case NODOKA_BC_LTEQ:
            case NODOKA_BC_EQ:
            case NODOKA_BC_S_EQ: {
                nodoka_data *sp0 = POP();
                nodoka_data *sp1 = POP();
                if (sp0 && sp1) {
                    bool result;
                    switch (bc) {
                        case NODOKA_BC_LT: result = nodoka_absRelComp(sp1, sp0) == 1; break;
                        case NODOKA_BC_LTEQ: result = nodoka_absRelComp(sp0, sp1) == 0; break;
                        case NODOKA_BC_EQ: result = nodoka_absEqComp(sp1, sp0); break;
                        case NODOKA_BC_S_EQ: result = nodoka_strictEqComp(sp1, sp0); break;
                        default: assert(0);
                    }
                    PUSH(result ? nodoka_true : nodoka_false);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, result ? NODOKA_BC_TRUE : NODOKA_BC_FALSE);
                    continue;
                } else {
                    PUSH(NULL);
                    break;
                }
                break;
            }
            default: {
                assert(0);
                break;
            }
        }
        nodoka_emitBytecode(target, bc);
    }
    free(typeStack);
    return modified;
}

