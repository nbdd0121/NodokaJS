#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

int main() {
    /* Read js from disk */
    FILE *f = fopen("./test.js", "r+");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    char *buffer = malloc(size + 1);
    rewind(f);
    size_t numread=fread(buffer, sizeof(char), size, f);
    assert(numread==size);
    buffer[size] = 0;

    

    return 0;
}

