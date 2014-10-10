#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"
#include "c/stdlib.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/pass.h"

bool nodoka_intraPcr(nodoka_code_emitter *source, nodoka_intraPcrPass pass) {
    nodoka_code_emitter *temp = nodoka_newCodeEmitter();

    size_t size = source->bytecodeLength;
    uint16_t *labelMap = malloc(sizeof(uint16_t) * size);
    for (int i = 0; i < size; i++) {
        labelMap[i] = 0xFFFF;
    }

    /* Mark all branch nodes (target or origin of a branch instruction) */
    for (size_t i = 0; i < size;) {
        enum nodoka_bytecode bc = nodoka_pass_fetch8(source, &i);
        switch (bc) {
            case NODOKA_BC_LOAD_STR: i += 2; break;
            case NODOKA_BC_LOAD_NUM: i += 8; break;
            case NODOKA_BC_JMP:
            case NODOKA_BC_JT: {
                labelMap[i - 1] = 0;
                uint16_t offset = nodoka_pass_fetch16(source, &i);
                labelMap[offset] = 0;
                break;
            }
            default: break;
        }
    }

    bool mod = false;
    size_t start = 0;
    /* Transform code and build a map from label to label */
    while (start < size) {
        size_t end;
        for (end = start + 1; end < size; end++) {
            if (labelMap[end] != 0xFFFF) {
                break;
            }
        }
        switch (source->bytecode[start]) {
            case NODOKA_BC_JMP:
            case NODOKA_BC_JT: {
                labelMap[start] = temp->bytecodeLength;
                nodoka_emitBytecode(temp, source->bytecode[start], NULL);
                mod |= pass(source, temp, start + 3, end);
                break;
            }
            default: {
                labelMap[start] = temp->bytecodeLength;
                mod |= pass(source, temp, start, end);
                break;
            }
        }
        start = end;
    }

    /* Fill in the corresponding label */
    for (int i = 0; i < size; i++) {
        if (labelMap[i] != 0xFFFF) {
            switch (source->bytecode[i]) {
                case NODOKA_BC_JMP:
                case NODOKA_BC_JT: {
                    nodoka_relocatable rel = labelMap[i] + 1;
                    nodoka_label prevRel = (source->bytecode[i + 1] << 8) | source->bytecode[i + 2];
                    nodoka_relocate(temp, rel, labelMap[prevRel]);
                    break;
                }
                default: break;
            }
        }
    }

    /* Switch the emitter */
    nodoka_xchgEmitter(source, temp);
    nodoka_freeEmitter(temp);

    free(labelMap);

    return mod;
}
