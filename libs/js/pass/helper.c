#include "js/pass.h"

uint8_t nodoka_pass_fetch8(nodoka_code_emitter *code, size_t *ptr) {
    return code->bytecode[(*ptr)++];
}

uint16_t nodoka_pass_fetch16(nodoka_code_emitter *context, size_t *ptr) {
    uint16_t ret = nodoka_pass_fetch8(context, ptr) << 8;
    ret |= nodoka_pass_fetch8(context, ptr);
    return ret;
}

uint32_t nodoka_pass_fetch32(nodoka_code_emitter *context, size_t *ptr) {
    uint32_t ret = nodoka_pass_fetch16(context, ptr) << 16;
    ret |= nodoka_pass_fetch16(context, ptr);
    return ret;
}

uint64_t nodoka_pass_fetch64(nodoka_code_emitter *context, size_t *ptr) {
    uint64_t ret = (uint64_t)nodoka_pass_fetch32(context, ptr) << 32;
    ret |= nodoka_pass_fetch32(context, ptr);
    return ret;
}