#ifndef JS_JS_H
#define JS_JS_H

#include "c/stdint.h"
#include "c/stdbool.h"
#include "c/assert.h"

#include "unicode/convert.h"

enum nodoka_data_type {
    NODOKA_NULL = 0x1,
    NODOKA_UNDEF = 0x2,
    NODOKA_BOOL = 0x4,
    NODOKA_NUMBER = 0x8,
    NODOKA_STRING = 0x10,
    NODOKA_OBJECT = 0x20,

    NODOKA_REFERENCE = 0x40,

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
typedef struct nodoka_context nodoka_context;

nodoka_code_emitter *nodoka_newCodeEmitter(void);
void nodoka_emitBytecode(nodoka_code_emitter *codeseg, uint8_t bc, ...);
nodoka_code *nodoka_packCode(nodoka_code_emitter *emitter);
nodoka_code_emitter *nodoka_unpackCode(nodoka_code *code);

nodoka_context *nodoka_newContext(nodoka_code *code);
void *nodoka_exec(nodoka_context *context);

bool nodoka_nopPass(nodoka_code_emitter *codeseg);
bool nodoka_peeholePass(nodoka_code_emitter *codeseg);
bool nodoka_convPass(nodoka_code_emitter *emitter);

void nodoka_printBytecode(nodoka_code *);

void nodoka_initConstant(void);
nodoka_data *nodoka_new_data(enum nodoka_data_type type);
nodoka_string *nodoka_new_string(utf16_string_t str);
nodoka_number *nodoka_newNumber(double val);

/* conversion.c */
nodoka_data *nodoka_toPrimitive(nodoka_data *value);
nodoka_data *nodoka_toBoolean(nodoka_data *value);
nodoka_number *nodoka_toNumber(nodoka_data *value);
int32_t nodoka_toInt32(nodoka_number *value);
uint32_t nodoka_toUint32(nodoka_number *value);
int16_t nodoka_toInt16(nodoka_number *value);
nodoka_string *nodoka_toString(nodoka_data *value);

/* vm/string.c */
nodoka_string *nodoka_concatString(nodoka_string *lstr, nodoka_string *rstr);

/* bcloader.c */
char *nodoka_readFile(char *path, size_t *sizePtr);
nodoka_code *nodoka_loadBytecode(char *path);

extern nodoka_data *nodoka_null;
extern nodoka_data *nodoka_undefined;
extern nodoka_data *nodoka_true;
extern nodoka_data *nodoka_false;
extern nodoka_number *nodoka_nan;
extern nodoka_number *nodoka_zero;
extern nodoka_number *nodoka_one;
extern nodoka_string *nodoka_nullStr;
extern nodoka_string *nodoka_undefStr;
extern nodoka_string *nodoka_trueStr;
extern nodoka_string *nodoka_falseStr;
extern nodoka_string *nodoka_nanStr;
extern nodoka_string *nodoka_infStr;
extern nodoka_string *nodoka_negInfStr;
extern nodoka_string *nodoka_zeroStr;

#define NODOKA_TYPE(data) (((nodoka_data*)(data))->type)
#define assertType(data, type) do{enum nodoka_data_type __type=NODOKA_TYPE(data);assert((__type&(type))==__type);}while(0)
#define assertPrimitive(data) assertType(data, NODOKA_UNDEF|NODOKA_NULL|NODOKA_BOOL|NODOKA_NUMBER|NODOKA_STRING)
#define assertNumber(data) assertType(data, NODOKA_NUMBER)
#define assertBoolean(data) assertType(data, NODOKA_BOOL)

#endif
