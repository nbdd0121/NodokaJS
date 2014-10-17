#include "c/math.h"
#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"

#include "unicode/hash.h"
#include "data-struct/hashmap.h"

hashmap_t *utf8Hashmap;
hashmap_t *strHashmap;

void nodoka_initStringPool(void) {
    utf8Hashmap = hashmap_new_string(11);
    strHashmap = hashmap_new(unicode_utf16Hash, unicode_utf16Cmp, 11);
}

nodoka_string *nodoka_new_string(utf16_string_t str) {
    nodoka_string *get = hashmap_get(strHashmap, &str);
    if (get) {
        free(str.str);
        return get;
    }
    nodoka_string *string = (nodoka_string *)nodoka_new_data(NODOKA_STRING);
    string->value = str;
    hashmap_put(strHashmap, &string->value, string);
    return string;
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

nodoka_string *nodoka_concatString(nodoka_string *lstr, nodoka_string *rstr) {
    size_t len = lstr->value.len + rstr->value.len;
    uint16_t *str = malloc(len * sizeof(uint16_t));
    memcpy(str, lstr->value.str, lstr->value.len * sizeof(uint16_t));
    memcpy(str + lstr->value.len, rstr->value.str, rstr->value.len * sizeof(uint16_t));
    return nodoka_new_string((utf16_string_t) {
        .str = str, .len = len
    });
}
