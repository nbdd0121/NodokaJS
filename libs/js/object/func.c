#include "c/assert.h"

#include "js/object.h"

#include "unicode/hash.h"

enum nodoka_completion nodoka_call(nodoka_global *G, nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    return O->call(G, O, this, ret, argc, argv);
}

enum nodoka_completion nodoka_construct(nodoka_global *G, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv) {
    return O->construct(G, O, ret, argc, argv);
}
