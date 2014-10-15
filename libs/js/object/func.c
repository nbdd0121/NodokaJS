#include "c/assert.h"

#include "js/object.h"

#include "unicode/hash.h"

enum nodoka_completion nodoka_call(nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    return O->call(O, this, ret, argc, argv);
}

nodoka_object *nodoka_construct(nodoka_object *O, int argc, nodoka_data **argv) {
    return O->construct(O, argc, argv);
}
