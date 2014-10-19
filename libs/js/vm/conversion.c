#include "c/math.h"
#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"
#include "js/object.h"

nodoka_data *nodoka_toPrimitive(nodoka_data *value) {
    switch (value->type) {
        case NODOKA_UNDEF:
        case NODOKA_NULL:
        case NODOKA_BOOL:
        case NODOKA_NUMBER:
        case NODOKA_STRING:
            return value;
        default:
            assert(0);
    }
}

nodoka_data *nodoka_toBoolean(nodoka_data *value) {
    switch (value->type) {
        case NODOKA_UNDEF:
        case NODOKA_NULL:
            return nodoka_false;
        case NODOKA_BOOL:
            return value;
        case NODOKA_NUMBER: {
            nodoka_number *num = (nodoka_number *)value;
            if (num->value == 0.0 || isnan(num->value)) {
                return nodoka_false;
            } else {
                return nodoka_true;
            }
        }
        case NODOKA_STRING: {
            nodoka_string *str = (nodoka_string *)value;
            if (str->value.len) {
                return nodoka_true;
            } else {
                return nodoka_false;
            }
        }
        case NODOKA_OBJECT:
            return nodoka_true;
        default: assert(0);
    }
}

nodoka_number *nodoka_toNumber(nodoka_data *value) {
    switch (value->type) {
        case NODOKA_UNDEF:
            return nodoka_nan;
        case NODOKA_NULL:
            return nodoka_zero;
        case NODOKA_BOOL:
            if (value == nodoka_true) {
                return nodoka_one;
            } else {
                return nodoka_zero;
            }
        case NODOKA_NUMBER:
            return (nodoka_number *)value;
        case NODOKA_STRING: {
            return nodoka_str2num((nodoka_string *)value);
        }
        default: assert(0);
    }
}

int32_t nodoka_toInt32(nodoka_number *value) {
    double number = value->value;
    if (isnan(number) || isinf(number) || number == 0) {
        return 0;
    }
    double posInt = (number > 0 ? 1 : -1) * floor(fabs(number));
    int32_t int32bit = (int32_t)posInt;
    return int32bit;
}

uint32_t nodoka_toUint32(nodoka_number *value) {
    return (uint32_t)nodoka_toInt32(value);
}

uint16_t nodoka_toUint16(nodoka_number *value) {
    return (uint16_t)nodoka_toInt32(value);
}

nodoka_string *nodoka_toString(nodoka_context *C, nodoka_data *value) {
    switch (value->type) {
        case NODOKA_UNDEF:
            return nodoka_undefStr;
        case NODOKA_NULL:
            return nodoka_nullStr;
        case NODOKA_BOOL:
            if (value == nodoka_true) {
                return nodoka_trueStr;
            } else {
                return nodoka_falseStr;
            }
        case NODOKA_NUMBER: {
            return nodoka_num2str(((nodoka_number *)value)->value);
        }
        case NODOKA_STRING:
            return (nodoka_string *)value;
        case NODOKA_OBJECT: {
            return nodoka_toString(C, nodoka_defaultValue(C, (nodoka_object *)value, NODOKA_STRING));
        }
        default:
            assert(0);
    }
}

nodoka_object *nodoka_toObject(nodoka_context *C, nodoka_data *value) {
    switch (value->type) {
        case NODOKA_STRING: {
            nodoka_data *ret;
            nodoka_construct(C, C->global->String, &ret, 1, (nodoka_data*[1]) {
                value
            });
            return (nodoka_object *)ret;
        }
        case NODOKA_OBJECT: return (nodoka_object *)value;
        default: assert(0);
    }
}

