#include "c/stdio.h"
#include "c/assert.h"
#include "c/string.h"

#include "js/js.h"
#include "js/bytecode.h"

static uint8_t FROM0[] = {
    NODOKA_BC_XCHG,
    NODOKA_BC_XCHG
};

static uint8_t TO0[] = {
    NODOKA_BC_NOP,
    NODOKA_BC_NOP,
};

struct seq {
    size_t len;
    uint8_t *from;
    uint8_t *to;
} sequences[] = {
    {sizeof(FROM0), FROM0, TO0}
};

#define SEQ_NUM (sizeof(sequences)/sizeof(sequences[0]))

bool nodoka_peeholePass(nodoka_code_emitter *codeseg) {
    bool mod = false;
    for (int i = 0; i < codeseg->bytecodeLength; i++) {
        enum nodoka_bytecode bc = codeseg->bytecode[i];
        for (int j = 0; j < SEQ_NUM; j++) {
            struct seq *s = &sequences[j];
            if (bc == s->from[0] &&
                    i + s->len <= codeseg->bytecodeLength) {
                if (memcmp(codeseg->bytecode + i, s->from, s->len) == 0) {
                    memcpy(codeseg->bytecode + i, s->to, s->len);
                    i += s->len - 1;
                    mod = true;
                    continue;
                }
            }
        }
    }
    return mod;
}
