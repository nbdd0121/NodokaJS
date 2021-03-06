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

    NODOKA_PROPERTY = 0x80,
    NODOKA_CODE = 0x100,

    NODOKA_ENV = 0x200
};

typedef struct {
    enum nodoka_data_type type;
} nodoka_data;

typedef struct {
    nodoka_data base;
    double value;
} nodoka_number;

typedef struct {
    nodoka_data base;
    utf16_string_t value;
    nodoka_number *numberCache;
} nodoka_string;

typedef struct {
    nodoka_data class_base;
    nodoka_data *base;
    nodoka_string *name;
} nodoka_reference;

typedef struct nodoka_object nodoka_object;
typedef struct nodoka_prop_desc nodoka_prop_desc;
typedef struct nodoka_code nodoka_code;
typedef struct nodoka_code_emitter nodoka_code_emitter;
typedef struct nodoka_context nodoka_context;
typedef struct nodoka_envRec nodoka_envRec;

enum nodoka_completion {
    NODOKA_COMPLETION_NORMAL,
    NODOKA_COMPLETION_RETURN,
    NODOKA_COMPLETION_THROW,
} nodoka_completion;

typedef struct nodoka_global {
    nodoka_object *global;
    nodoka_object *object;
    nodoka_object *Object_prototype;
    nodoka_object *function;
    nodoka_object *Function_prototype;
    nodoka_object *Array;
    nodoka_object *Array_prototype;
    nodoka_object *String;
    nodoka_object *String_prototype;
    nodoka_object *Error;
    nodoka_object *Error_prototype;
    nodoka_object *ReferenceError;
    nodoka_object *ReferenceError_prototype;
    nodoka_object *TypeError;
    nodoka_object *TypeError_prototype;
} nodoka_global;

struct nodoka_config {
    bool peehole;
    bool conv;
    bool fold;
};

enum nodoka_completion nodoka_exec(nodoka_context *context, nodoka_data **ret);


int8_t nodoka_absRelComp(nodoka_data *sp1, nodoka_data *sp0);
bool nodoka_sameValue(nodoka_data *x, nodoka_data *y);
bool nodoka_strictEqComp(nodoka_data *x, nodoka_data *y);
bool nodoka_absEqComp(nodoka_data *x, nodoka_data *y);

void nodoka_printBytecode(nodoka_code *, int indent);

void nodoka_initConstant(void);
void nodoka_initStringPool(void);

nodoka_data *nodoka_new_data(enum nodoka_data_type type);
nodoka_string *nodoka_new_string(utf16_string_t str);
nodoka_number *nodoka_newNumber(double val);
nodoka_reference *nodoka_newReference(nodoka_data *base, nodoka_string *name);


/* string.c */
nodoka_string *nodoka_newStringFromUtf8(char *str);

/* conversion.c */
nodoka_string *nodoka_newStringFromDouble(double value);

nodoka_data *nodoka_toPrimitive(nodoka_data *value);
nodoka_data *nodoka_toBoolean(nodoka_data *value);
nodoka_number *nodoka_toNumber(nodoka_data *value);
int32_t nodoka_toInt32(nodoka_number *value);
uint32_t nodoka_toUint32(nodoka_number *value);
uint16_t nodoka_toUint16(nodoka_number *value);
nodoka_string *nodoka_toString(nodoka_context *C, nodoka_data *value);
nodoka_object *nodoka_toObject(nodoka_context *C, nodoka_data *value);

nodoka_number *nodoka_str2num(nodoka_string *str);
nodoka_string *nodoka_num2str(double val);

/* vm/string.c */
nodoka_string *nodoka_concatString(size_t i, ...);
nodoka_string *nodoka_newStringDup(utf16_string_t str);

/* bcloader.c */
char *nodoka_readFile(char *path, size_t *sizePtr);
nodoka_code *nodoka_loadBytecode(char *path);

int nodoka_compareString(void *a, void *b);
int nodoka_hashString(void *a);

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
#define assertString(data) assertType(data, NODOKA_STRING)
#define assertBoolean(data) assertType(data, NODOKA_BOOL)

extern struct nodoka_config nodoka_config;

#endif
