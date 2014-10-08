#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"

#include "js/js.h"
#include "js/bytecode.h"

bool nodoka_nopPass(nodoka_code_emitter *codeseg) {
    int offset = 0;
    for (int i = 0; i < codeseg->bytecodeLength; i++) {
        enum nodoka_bytecode bc = codeseg->bytecode[i];
        if (bc == NODOKA_BC_NOP) {
            offset++;
            continue;
        }
        if (offset) {
            codeseg->bytecode[i - offset] = bc;
        }
    }
    codeseg->bytecodeLength -= offset;
    return !!offset;
}
