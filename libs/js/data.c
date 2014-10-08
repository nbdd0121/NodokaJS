#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"
#include "js/bytecode.h"

nodoka_data *nodoka_new_data(enum nodoka_data_type type) {
    size_t size;
    switch (type) {
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