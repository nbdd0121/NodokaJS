#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"

#include "js/js.h"
#include "js/bytecode.h"

bool nodoka_nopPass(nodoka_code_emitter *codeseg) {
    int offset = 0;
    for (int i = 0; i < codeseg->bytecodeLength; i++) {
        enum nodoka_bytecode bc = codeseg->bytecode[i];
        switch (bc) {
            case NODOKA_BC_LOAD_STR: memmove(&codeseg->bytecode[i - offset], &codeseg->bytecode[i], 3); i += 2; break;
            case NODOKA_BC_LOAD_NUM: memmove(&codeseg->bytecode[i - offset], &codeseg->bytecode[i], 9); i += 8; break;
            case NODOKA_BC_NOP: offset++; break;
            default: codeseg->bytecode[i - offset] = bc; break;
        }
    }
    codeseg->bytecodeLength -= offset;
    return !!offset;
}
