#include "c/math.h"
#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"

nodoka_string *nodoka_concatString(nodoka_string *lstr, nodoka_string *rstr) {
    size_t len = lstr->value.len + rstr->value.len;
    uint16_t *str = malloc(len * sizeof(uint16_t));
    memcpy(str, lstr->value.str, lstr->value.len * sizeof(uint16_t));
    memcpy(str + lstr->value.len, rstr->value.str, rstr->value.len * sizeof(uint16_t));
    return nodoka_new_string((utf16_string_t) {
        .str = str, .len = len
    });
}
