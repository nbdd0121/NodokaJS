#ifndef JS_PASS_H
#define JS_PASS_H

#include "c/stdint.h"
#include "js/bytecode.h"

uint8_t nodoka_pass_fetch8(nodoka_code_emitter *context, size_t *ptr);
uint16_t nodoka_pass_fetch16(nodoka_code_emitter *context, size_t *ptr);
uint32_t nodoka_pass_fetch32(nodoka_code_emitter *context, size_t *ptr);
uint64_t nodoka_pass_fetch64(nodoka_code_emitter *context, size_t *ptr);

typedef bool (*nodoka_intraPcrPass)(nodoka_code_emitter *source, nodoka_code_emitter *target, size_t start, size_t end);
bool nodoka_intraPcr(nodoka_code_emitter *source, nodoka_intraPcrPass pass);

bool nodoka_peeholePass(nodoka_code_emitter *codeseg, nodoka_code_emitter *target, size_t start, size_t end);
bool nodoka_convPass(nodoka_code_emitter *emitter, nodoka_code_emitter *target, size_t start, size_t end);

#endif