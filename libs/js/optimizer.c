#include "js/bytecode.h"
#include "js/pass.h"

void nodoka_optimizer(nodoka_code_emitter *emitter) {
    /* Optimize */
    for (int i = 0; i < 10; i++) {
        bool mod = false;
        if (nodoka_config.conv) {
            mod |= nodoka_intraPcr(emitter, nodoka_convPass);
        }
        if (nodoka_config.fold) {
            mod |= nodoka_intraPcr(emitter, nodoka_foldPass);
        }
        if (nodoka_config.peehole) {
            mod |= nodoka_intraPcr(emitter, nodoka_peeholePass);
        }
        if (!mod) {
            break;
        }
    }
}