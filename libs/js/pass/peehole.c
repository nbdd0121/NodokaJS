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
            case NODOKA_BC_LOAD_STR: {
                uint16_t offset = nodoka_pass_fetch16(emitter, &i);
                /* LOAD_STR [imm16] POP can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_POP) {
                    i++;
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
                } else {
                    nodoka_emitBytecode(target, bc, val);
                }
                continue;
            }
            case NODOKA_BC_XCHG: {
                /* XCHG XCHG can be removed with no side-effect */
                if (i < end && emitter->bytecode[i] == NODOKA_BC_XCHG) {
                    i++;
                    continue;
                }
                break;
            }
            case NODOKA_BC_NOP: {
                continue;
            }
            default:
                break;
        }
        nodoka_emitBytecode(target, bc);
    }
    return mod;
}
