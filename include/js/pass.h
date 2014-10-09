#ifndef JS_PASS_H
#define JS_PASS_H

#include "c/stdint.h"
#include "js/bytecode.h"

uint8_t nodoka_pass_fetch8(nodoka_code_emitter *context, size_t *ptr);
uint16_t nodoka_pass_fetch16(nodoka_code_emitter *context, size_t *ptr);
uint32_t nodoka_pass_fetch32(nodoka_code_emitter *context, size_t *ptr);
uint64_t nodoka_pass_fetch64(nodoka_code_emitter *context, size_t *ptr);

#endif