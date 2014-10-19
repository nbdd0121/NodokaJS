#include "c/math.h"
#include "c/assert.h"
#include "c/stdlib.h"
#include "c/stdarg.h"

#include "js/js.h"

#include "unicode/hash.h"
#include "data-struct/hashmap.h"
#include "util/double.h"

hashmap_t *utf8Hashmap;
hashmap_t *strHashmap;
hashmap_t *num2strMap;

int nodoka_compareNumber(void *a, void *b) {
    double x = *(double *)a;
    double y = *(double *)b;
    if ((x != x && y != y) || x == y) {
        return 0;
    }
    if (x > y) {
        return 1;
    } else {
        return -1;
    }
}

int nodoka_hashNumber(void *a) {
    double val = *(double *)a;
    if (val != val) {
        return 0;
    }
    uint64_t x = double2int(val);
    return (uint32_t)(x >> 32) ^ (uint32_t)x;
}

void nodoka_initStringPool(void) {
    utf8Hashmap = hashmap_new_string(11);
    strHashmap = hashmap_new(unicode_utf16Hash, unicode_utf16Cmp, 11);
    num2strMap = hashmap_new(nodoka_hashNumber, nodoka_compareNumber, 11);
}

nodoka_string *nodoka_new_string(utf16_string_t str) {
    nodoka_string *get = hashmap_get(strHashmap, &str);
    if (get) {
        free(str.str);
        return get;
    }
    nodoka_string *string = (nodoka_string *)nodoka_new_data(NODOKA_STRING);
    string->value = str;
    string->numberCache = NULL;
    hashmap_put(strHashmap, &string->value, string);
    return string;
}

nodoka_string *nodoka_num2str(double val) {
    nodoka_string *ret = hashmap_get(num2strMap, &val);
    if (ret) {
        return ret;
    }
    double *ptr = malloc(sizeof(double));
    *ptr = val;
    ret = nodoka_newStringFromDouble(val);
    hashmap_put(num2strMap, ptr, ret);
    return ret;
}

nodoka_string *nodoka_newStringDup(utf16_string_t str) {
    utf16_string_t dup = {
        .len = str.len,
        .str = malloc(sizeof(uint16_t) * str.len)
    };
    memcpy(dup.str, str.str, sizeof(uint16_t) * str.len);
    return nodoka_new_string(dup);
}

nodoka_string *nodoka_newStringFromUtf8(char *str) {
    nodoka_string *get = hashmap_get(utf8Hashmap, str);
    if (get) {
        return get;
    }
    nodoka_string *string = nodoka_new_string(unicode_toUtf16(UTF8_STRING(str)));
    hashmap_put(utf8Hashmap, str, get);
    return string;
}

nodoka_string *nodoka_concatString(size_t num, ...) {
    va_list ap;
    va_start(ap, num);
    nodoka_string *strs[num];
    size_t len = 0;
    for (size_t i = 0; i < num; i++) {
        strs[i] = va_arg(ap, nodoka_string *);
        len += strs[i]->value.len;
    }
    uint16_t *str = malloc(len * sizeof(uint16_t));
    size_t ptr = 0;
    for (size_t i = 0; i < num; i++) {
        memcpy(str + ptr, strs[i]->value.str, strs[i]->value.len * sizeof(uint16_t));
        ptr += strs[i]->value.len;
    }
    va_end(ap);
    return nodoka_new_string((utf16_string_t) {
        .str = str, .len = len
    });
}
