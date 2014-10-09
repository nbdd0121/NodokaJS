#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"

#include "js/js.h"
#include "js/bytecode.h"

#define SEQ_NUM (sizeof(sequences)/sizeof(sequences[0]))

bool nodoka_peeholePass(nodoka_code_emitter *codeseg) {
    bool mod = false;
    for (int i = 0; i < codeseg->bytecodeLength; i++) {
        enum nodoka_bytecode bc = codeseg->bytecode[i];
        switch (bc) {
            case NODOKA_BC_LOAD_STR: i += 2; break;
            case NODOKA_BC_LOAD_NUM: i += 8; break;
            case NODOKA_BC_XCHG: {
                if (codeseg->bytecode[i + 1]) {
                    codeseg->bytecode[i] = NODOKA_BC_NOP;
                    codeseg->bytecode[i + 1] = NODOKA_BC_NOP;
                    i++;
                }
                break;
            }
            default: break;
        }
    }
    return mod;
}
