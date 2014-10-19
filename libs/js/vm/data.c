#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/object.h"

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
    nodoka_initStringPool();
    nodoka_nullStr = nodoka_newStringFromUtf8("null");
    nodoka_undefStr = nodoka_newStringFromUtf8("undefined");
    nodoka_trueStr = nodoka_newStringFromUtf8("true");
    nodoka_falseStr = nodoka_newStringFromUtf8("false");
    nodoka_nanStr = nodoka_newStringFromUtf8("NaN");
    nodoka_infStr = nodoka_newStringFromUtf8("Infinity");
    nodoka_negInfStr = nodoka_newStringFromUtf8("-Infinity");
    nodoka_zeroStr = nodoka_newStringFromUtf8("0");
}

nodoka_data *nodoka_new_data(enum nodoka_data_type type) {
    size_t size;
    switch (type) {
        case NODOKA_NULL:
        case NODOKA_UNDEF:
        case NODOKA_BOOL: size = sizeof(nodoka_data); break;
        case NODOKA_NUMBER: size = sizeof(nodoka_number); break;
        case NODOKA_STRING: size = sizeof(nodoka_string); break;
        case NODOKA_OBJECT: size = sizeof(nodoka_object); break;

        case NODOKA_REFERENCE: size = sizeof(nodoka_reference); break;
        case NODOKA_PROPERTY: size = sizeof(nodoka_prop_desc); break;
        case NODOKA_CODE: size = sizeof(nodoka_code); break;
        case NODOKA_ENV: size = sizeof(nodoka_envRec); break;
        default: assert(0);
    }
    nodoka_data *data = malloc(size);
    data->type = type;
    return data;
}

nodoka_number *nodoka_newNumber(double val) {
    nodoka_number *number = (nodoka_number *)nodoka_new_data(NODOKA_NUMBER);
    number->value = val;
    return number;
}

nodoka_reference *nodoka_newReference(nodoka_data *base, nodoka_string *name) {
    nodoka_reference *ref = (nodoka_reference *)nodoka_new_data(NODOKA_REFERENCE);
    ref->base = base;
    ref->name = name;
    return ref;
}

nodoka_prop_desc *nodoka_newPropertyDesc(void) {
    nodoka_prop_desc *propDesc = (nodoka_prop_desc *)nodoka_new_data(NODOKA_PROPERTY);
    propDesc->value = NULL;
    propDesc->get = NULL;
    propDesc->set = NULL;
    propDesc->writable = NULL;
    propDesc->enumerable = NULL;
    propDesc->configurable = NULL;
    return propDesc;
}

