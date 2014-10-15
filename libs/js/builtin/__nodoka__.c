#include "c/math.h"

#include "js/object.h"

nodoka_prop_desc *nodoka_createDataDesc(nodoka_data *val, bool writable, bool enumerable, bool configurable);

static nodoka_object *createFunc() {
    return nodoka_newObject();
}

static enum nodoka_completion print_native(nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    for (int i = 0; i < argc; i++) {
        nodoka_string *str = nodoka_toString(argv[i]);
        unicode_putUtf16(str->value);
    }
    *ret = nodoka_undefined;
    return NODOKA_COMPLETION_RETURN;
}

static enum nodoka_completion nodoka_colorDir(nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    for (int i = 0; i < argc; i++) {
        nodoka_data *data = argv[i];
        switch (data->type) {
            case NODOKA_UNDEF: printf("\033[0;37mundefined"); break;
            case NODOKA_NULL: printf("\033[1;39mnull"); break;
            case NODOKA_NUMBER: printf("\033[0;33m"); unicode_putUtf16(nodoka_toString(data)->value); break;
            case NODOKA_STRING: printf("\033[0;32m'"); unicode_putUtf16(((nodoka_string *)data)->value); printf("'"); break;
            case NODOKA_OBJECT:
            default: assert(0);
        }
        printf("\033[0m ");
    }
    printf("\n");
    *ret = nodoka_undefined;
    return NODOKA_COMPLETION_RETURN;
}

nodoka_object *nodoka_newGlobal_nodoka(void) {
    nodoka_object *nodoka = nodoka_newObject();
    nodoka_object *print = createFunc();
    print->call = print_native;
    nodoka_defineOwnProperty(nodoka,
                             nodoka_newStringFromUtf8("print"),
                             nodoka_createDataDesc((nodoka_data *)print, false, false, false),
                             true);
    nodoka_object *colorDir = createFunc();
    colorDir->call = nodoka_colorDir;
    nodoka_defineOwnProperty(nodoka,
                             nodoka_newStringFromUtf8("colorDir"),
                             nodoka_createDataDesc((nodoka_data *)colorDir, false, false, false),
                             true);
    return nodoka;
}