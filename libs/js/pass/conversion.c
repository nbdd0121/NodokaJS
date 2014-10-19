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
        enum nodoka_data_type ret;\
        if(stackTop<=typeStack){\
            ret=0xFF;\
        }else{\
            ret=*(--stackTop);\
        }\
        ret;\
    })

#define PEEK() ({\
        enum nodoka_data_type ret;\
        if(stackTop<=typeStack){\
            ret=0xFF;\
        }else{\
            ret=*(stackTop-1);\
        }\
        ret;\
    })

static bool isPrim(enum nodoka_data_type x) {
    return (x & (NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING)) == x;
}

static bool canBeRef(enum nodoka_data_type x) {
    return !!(x & NODOKA_REFERENCE);
}

bool nodoka_convPass(nodoka_code_emitter *emitter, nodoka_code_emitter *target, size_t start, size_t end) {
    enum nodoka_data_type *typeStack = malloc(128);
    enum nodoka_data_type *stackTop = typeStack;
    enum nodoka_data_type *stackLimit = typeStack + 128;
    bool mod = false;
    for (size_t i = start; i < end;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(emitter, &i);
        switch (bc) {
            case NODOKA_BC_UNDEF: PUSH(NODOKA_UNDEF); break;
            case NODOKA_BC_NULL: PUSH(NODOKA_NULL); break;
            case NODOKA_BC_TRUE: PUSH(NODOKA_BOOL); break;
            case NODOKA_BC_FALSE: PUSH(NODOKA_BOOL); break;
            case NODOKA_BC_DECL: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                nodoka_emitBytecode(target, bc, emitter->stringPool[offset]);
                continue;
            }
            case NODOKA_BC_LOAD_STR: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                PUSH(NODOKA_STRING);
                nodoka_emitBytecode(target, bc, emitter->stringPool[offset]);
                continue;
            }
            case NODOKA_BC_LOAD_NUM: {
                double val = int2double(nodoka_pass_fetch64(emitter, &i));
                PUSH(NODOKA_NUMBER);
                nodoka_emitBytecode(target, bc, val);
                continue;
            }
            case NODOKA_BC_FUNC: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                PUSH(NODOKA_OBJECT);
                nodoka_emitBytecode(target, bc, emitter->codePool[offset]);
                continue;
            }
            case NODOKA_BC_LOAD_OBJ: {
                PUSH(NODOKA_OBJECT);
                break;
            }
            case NODOKA_BC_LOAD_ARR: {
                PUSH(NODOKA_OBJECT);
                break;
            }
            case NODOKA_BC_NOP: {
                mod = true;
                continue;
            }
            case NODOKA_BC_DUP: {
                enum nodoka_data_type sp0 = POP();
                PUSH(sp0);
                PUSH(sp0);
                break;
            }
            case NODOKA_BC_POP: POP(); break;
            case NODOKA_BC_XCHG: {
                enum nodoka_data_type sp0 = POP();
                enum nodoka_data_type sp1 = POP();
                PUSH(sp0);
                PUSH(sp1);
                break;
            }
            case NODOKA_BC_XCHG3: {
                enum nodoka_data_type sp0 = POP();
                enum nodoka_data_type sp1 = POP();
                enum nodoka_data_type sp2 = POP();
                PUSH(sp0);
                PUSH(sp2);
                PUSH(sp1);
                break;
            }
            case NODOKA_BC_RET:
            case NODOKA_BC_THROW: {
                if (i != end) {
                    i = end;
                    mod = true;
                }
                break;
            }
            case NODOKA_BC_NOCATCH:
                break;
            case NODOKA_BC_TRY: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                PUSH(NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING | NODOKA_OBJECT);
                nodoka_emitBytecode(target, bc, emitter->codePool[offset]);
                continue;
            }
            case NODOKA_BC_THIS: PUSH(NODOKA_OBJECT); break;
            case NODOKA_BC_PRIM: {
                enum nodoka_data_type type = POP();
                if (isPrim(type)) {
                    PUSH(type);
                    mod = true;
                    continue;
                } else {
                    PUSH(NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING);
                    break;
                }
            }
            case NODOKA_BC_BOOL: {
                enum nodoka_data_type type = POP();
                PUSH(NODOKA_BOOL);
                if (type == NODOKA_BOOL) {
                    mod = true;
                    continue;
                } else {
                    break;
                }
            }
            case NODOKA_BC_NUM: {
                enum nodoka_data_type type = POP();
                PUSH(NODOKA_NUMBER);
                if (type == NODOKA_NUMBER) {
                    mod = true;
                    continue;
                } else {
                    break;
                }
            }
            case NODOKA_BC_STR: {
                enum nodoka_data_type type = POP();
                PUSH(NODOKA_STRING);
                if (type == NODOKA_STRING) {
                    mod = true;
                    continue;
                } else {
                    break;
                }
            }
            case NODOKA_BC_REF: {
                POP();
                POP();
                PUSH(NODOKA_REFERENCE);
                break;
            }
            case NODOKA_BC_ID: {
                POP();
                PUSH(NODOKA_REFERENCE);
                break;
            }
            case NODOKA_BC_GET: {
                enum nodoka_data_type type = POP();
                if (!canBeRef(type)) {
                    mod = true;
                    PUSH(type);
                    continue;
                } else {
                    PUSH(NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING | NODOKA_OBJECT);
                    break;
                }
            }
            case NODOKA_BC_PUT: {
                POP();
                POP();
                break;
            }
            case NODOKA_BC_DEL: {
                POP();
                PUSH(NODOKA_BOOL);
                break;
            }
            case NODOKA_BC_CALL: {
                uint8_t count = nodoka_pass_fetch8(emitter, &i);
                nodoka_emitBytecode(target, NODOKA_BC_CALL, count);
                for (int i = 0; i < count; i++) {
                    POP();
                }
                POP();
                PUSH(NODOKA_UNDEF | NODOKA_NULL | NODOKA_BOOL | NODOKA_NUMBER | NODOKA_STRING | NODOKA_OBJECT);
                continue;
            }
            case NODOKA_BC_NEW: {
                uint8_t count = nodoka_pass_fetch8(emitter, &i);
                nodoka_emitBytecode(target, NODOKA_BC_NEW, count);
                for (int i = 0; i < count; i++) {
                    POP();
                }
                POP();
                PUSH(NODOKA_OBJECT);
                continue;
            }
            case NODOKA_BC_TYPEOF: {
                enum nodoka_data_type type = POP();
                if (type == NODOKA_UNDEF) {
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, nodoka_newStringFromUtf8("undefined"));
                    continue;
                } else if (type == NODOKA_NULL) {
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, nodoka_newStringFromUtf8("object"));
                    continue;
                } else if (type == NODOKA_BOOL) {
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, nodoka_newStringFromUtf8("boolean"));
                    continue;
                } else if (type == NODOKA_NUMBER) {
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, nodoka_newStringFromUtf8("number"));
                    continue;
                } else if (type == NODOKA_STRING) {
                    nodoka_emitBytecode(target, NODOKA_BC_POP);
                    nodoka_emitBytecode(target, NODOKA_BC_LOAD_STR, nodoka_newStringFromUtf8("string"));
                    continue;
                } else {
                    PUSH(NODOKA_STRING);
                }
                break;
            }
            case NODOKA_BC_NEG:
            case NODOKA_BC_NOT:
            case NODOKA_BC_L_NOT:
                break;
            case NODOKA_BC_MUL:
            case NODOKA_BC_MOD:
            case NODOKA_BC_DIV:
            case NODOKA_BC_SUB:
            case NODOKA_BC_SHL:
            case NODOKA_BC_SHR:
            case NODOKA_BC_USHR:
            case NODOKA_BC_AND:
            case NODOKA_BC_OR:
            case NODOKA_BC_XOR: POP(); break;
            case NODOKA_BC_ADD: {
                POP();
                POP();
                PUSH(NODOKA_STRING | NODOKA_NUMBER);
                break;
            }
            case NODOKA_BC_LT:
            case NODOKA_BC_LTEQ:
            case NODOKA_BC_EQ:
            case NODOKA_BC_S_EQ: {
                POP();
                POP();
                PUSH(NODOKA_BOOL);
                break;
            }
            default: assert(0);
        }
        nodoka_emitBytecode(target, bc);
    }
    free(typeStack);
    return mod;
}

