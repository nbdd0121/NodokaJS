#ifndef JS_JS_H
#define JS_JS_H

#include "c/stdint.h"
#include "c/stdbool.h"

#include "unicode/convert.h"

enum nodoka_data_type {
    NODOKA_NULL = 0x1,
    NODOKA_UNDEF = 0x2,
    NODOKA_BOOL = 0x4,
    NODOKA_NUMBER = 0x8,
    NODOKA_STRING = 0x10,

    NODOKA_CODE = 0xFF,
};

typedef struct {
    uint8_t type;
} nodoka_data;

typedef struct {
    nodoka_data base;
    double value;
} nodoka_number;

typedef struct {
    nodoka_data base;
    utf16_string_t value;
} nodoka_string;

typedef struct nodoka_code nodoka_code;
typedef struct nodoka_code_emitter nodoka_code_emitter;

nodoka_code_emitter *nodoka_newCodeEmitter(void);
void nodoka_emitBytecode(nodoka_code_emitter *codeseg, uint8_t bc);
nodoka_code *nodoka_packCode(nodoka_code_emitter *emitter);

bool nodoka_nopPass(nodoka_code_emitter *codeseg);
bool nodoka_peeholePass(nodoka_code_emitter *codeseg);

void nodoka_printBytecode(nodoka_code *);

nodoka_code *nodoka_loadBytecode(char *path);

nodoka_data *nodoka_new_data(enum nodoka_data_type type);
nodoka_string *nodoka_new_string(utf16_string_t str);

#endif
