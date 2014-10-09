/**
 * Header file providing utility for double conversion
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef UTIL_DOUBLE_H
#define UTIL_DOUBLE_H

#include "c/stdint.h"

static inline double int2double(uint64_t val) {
    union {
        double d;
        uint64_t i;
    } data = {
        .i = val
    };
    return data.d;
}

static inline uint64_t double2int(double val) {
    union {
        double d;
        uint64_t i;
    } data = {
        .d = val
    };
    return data.i;
}


#endif
