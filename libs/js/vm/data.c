#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"
#include "js/bytecode.h"

nodoka_data *nodoka_null;
nodoka_data *nodoka_undefined;
nodoka_data *nodoka_true;
nodoka_data *nodoka_false;
nodoka_number *nodoka_nan;
nodoka_number *nodoka_zero;
nodoka_number *nodoka_one;
nodoka_string *nodoka_nullStr;
nodoka_string *nodoka_undefStr;
nodoka_string *nodoka_trueStr;
nodoka_string *nodoka_falseStr;
nodoka_string *nodoka_nanStr;
nodoka_string *nodoka_infStr;
nodoka_string *nodoka_negInfStr;
nodoka_string *nodoka_zeroStr;

void nodoka_initConstant(void) {
    nodoka_null = nodoka_new_data(NODOKA_NULL);
    nodoka_undefined = nodoka_new_data(NODOKA_UNDEF);
    nodoka_true = nodoka_new_data(NODOKA_BOOL);
    nodoka_false = nodoka_new_data(NODOKA_BOOL);
    nodoka_nan = nodoka_newNumber(0.0 / 0.0);
    nodoka_zero = nodoka_newNumber(0);
    nodoka_one = nodoka_newNumber(1);
    nodoka_nullStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("null")));
    nodoka_undefStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("undefined")));
    nodoka_trueStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("true")));
    nodoka_falseStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("false")));
    nodoka_nanStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("NaN")));
    nodoka_infStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("Infinity")));
    nodoka_negInfStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("-Infinity")));
    nodoka_zeroStr = nodoka_new_string(unicode_toUtf16(UTF8_STRING("0")));
}

nodoka_data *nodoka_new_data(enum nodoka_data_type type) {
    size_t size;
    switch (type) {
        case NODOKA_NULL:
        case NODOKA_UNDEF:
        case NODOKA_BOOL: size = sizeof(nodoka_data); break;
        case NODOKA_NUMBER: size = sizeof(nodoka_number); break;
        case NODOKA_STRING: size = sizeof(nodoka_string); break;
        case NODOKA_CODE: size = sizeof(nodoka_code); break;
        default: assert(0);
    }
    nodoka_data *data = malloc(size);
    data->type = type;
    return data;
}

nodoka_string *nodoka_new_string(utf16_string_t str) {
    nodoka_string *string = (nodoka_string *)nodoka_new_data(NODOKA_STRING);
    string->value = str;
    return string;
}

nodoka_number *nodoka_newNumber(double val) {
    nodoka_number *number = (nodoka_number *)nodoka_new_data(NODOKA_NUMBER);
    number->value = val;
    return number;
}