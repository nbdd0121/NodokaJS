#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/pass.h"

#include "util/double.h"

#define SEQ_NUM (sizeof(sequences)/sizeof(sequences[0]))

bool nodoka_peeholePass(nodoka_code_emitter *emitter, nodoka_code_emitter *target, size_t start, size_t end) {
    bool mod = false;
    for (size_t i = start; i < end;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(emitter, &i);
        switch (bc) {
            case NODOKA_BC_DECL: {
                //since decl are always the first nodes, having a decl pointing to a existing string means already declared
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                nodoka_string *str = emitter->stringPool[offset];
                int i;
                for (i = 0; i < target->strPoolLength; i++) {
                    if (target->stringPool[i] == str) {
                        break;
                    }
                }
                if (i == target->strPoolLength) {
                    nodoka_emitBytecode(target, bc, str);
                } else {
                    mod = true;
                }
                continue;
            }
            case NODOKA_BC_UNDEF: {
                /* UNDEF POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                    continue;
                }
                break;
            }
            case NODOKA_BC_NULL: {
                /* NULL POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                    continue;
                }
                break;
            }
            case NODOKA_BC_TRUE: {
                /* TRUE POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                    continue;
                }
                break;
            }
            case NODOKA_BC_FALSE: {
                /* FALSE POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                    continue;
                }
                break;
            }
            case NODOKA_BC_LOAD_STR: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                /* LOAD_STR [imm16] POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                } else {
                    nodoka_emitBytecode(target, bc, emitter->stringPool[offset]);
                }
                continue;
            }
            case NODOKA_BC_LOAD_NUM: {
                double val = int2double(nodoka_pass_fetch64(emitter, &i));
                /* LOAD_NUM [imm64] POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                    continue;
                } /*else if (i + 2 < end && emitter->bytecode[i] == NODOKA_BC_XCHG && emitter->bytecode[i + 2] == NODOKA_BC_XCHG) {
switch (emitter->bytecode[i + 1]) {
case NODOKA_BC_PRIM:
case NODOKA_BC_NUM: {
nodoka_emitBytecode(target, emitter->bytecode[i + 1]);
nodoka_emitBytecode(target, bc, val);
i += 3;
mod = true;
continue;
}
}
}*/
                nodoka_emitBytecode(target, bc, val);
                continue;
            }
            case NODOKA_BC_FUNC: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                nodoka_emitBytecode(target, bc, emitter->codePool[offset]);
                continue;
            }
            case NODOKA_BC_TRY: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                nodoka_emitBytecode(target, bc, emitter->codePool[offset]);
                continue;
            }
            case NODOKA_BC_CALL:
            case NODOKA_BC_NEW: {
                uint8_t count = nodoka_pass_fetch8(emitter, &i);
                nodoka_emitBytecode(target, bc, count);
                continue;
            }
            case NODOKA_BC_XCHG: {
                /* XCHG XCHG can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_XCHG) {
                    i++;
                    mod = true;
                    continue;
                } else if (i + 1 < end && emitter->bytecode[i] == NODOKA_BC_POP && emitter->bytecode[i + 1] == NODOKA_BC_POP) {
                    /* XCHG POP POP can be reduced to POP POP with no side-effect */
                    mod = true;
                    continue;
                }
                break;
            }
            case NODOKA_BC_DUP: {
                /* DUP POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
                    mod = true;
                    continue;
                }
                if (i + 2 < end &&
                        emitter->bytecode[i] == NODOKA_BC_XCHG3 &&
                        emitter->bytecode[i + 1] == NODOKA_BC_PUT &&
                        emitter->bytecode[i + 2] == NODOKA_BC_POP) {
                    i += 3;
                    mod = true;
                    nodoka_emitBytecode(target, NODOKA_BC_PUT);
                    continue;
                }
                break;
            }
            case NODOKA_BC_NOP: {
                mod = true;
                continue;
            }
            case NODOKA_BC_L_NOT: {
                /* L_NOT L_NOT can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_L_NOT) {
                    i++;
                    mod = true;
                    continue;
                }
                break;
            }
            case NODOKA_BC_RET:
            case NODOKA_BC_THROW: {
                if (i != end) {
                    mod = true;
                }
                nodoka_emitBytecode(target, bc);
                return mod;
            }
            default:
                break;
        }
        nodoka_emitBytecode(target, bc);
    }
    return mod;
}
