#include "c/math.h"
#include "c/assert.h"
#include "c/stdlib.h"

#include "js/js.h"
#include "js/object.h"

#define EPISILON 0.00001

#ifndef __X86_64__

static uint32_t div64(uint64_t *num, uint32_t base) {
    uint64_t rem = *num;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64_t) high << 32;
        rem -= (uint64_t) (high * base) << 32;
    }

    while ((int64_t)b > 0 && b < rem) {
        b = b + b;
        d = d + d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *num = res;
    return rem;
}

#else

static uint32_t div64(uint64_t *num, uint32_t base) {
    uint32_t mod = *num % base;
    *num = *num / base;
    return mod;
}

#endif


static void sepFloatNum(double val, uint64_t *sPtr, int16_t *nPtr, uint16_t *kPtr) {
    int16_t N = (int16_t)floor(log10(val)) + 1;
    if (N > 0) {
        val /= pow(10, N);
    } else {
        val *= pow(10, -N);
    }

    uint64_t S = 0;
    uint16_t K = 0;

    while (val > EPISILON && val < 1 - EPISILON && K < 20) {
        val *= 10.;
        uint64_t n = (uint64_t)val;
        S = S * 10 + n;
        val -= n;
        K++;
    }
    if (val >= 1 - EPISILON) {
        S++;
        if (K == 0)
            K++;
    }
    *sPtr = S;
    *nPtr = N;
    *kPtr = K;
}

static uint8_t countLen(uint16_t N) {
    if (N >= 1000) {
        return 4;
    } else if (N >= 100) {
        return 3;
    } else if (N >= 10 ) {
        return 2;
    } else {
        return 1;
    }
}

nodoka_string *nodoka_newStringFromDouble(double value) {
    if (isnan(value)) {
        return nodoka_nanStr;
    } else if (value == 0.0) {
        return nodoka_zeroStr;
    } else if (isinf(value)) {
        if (value > 0) {
            return nodoka_infStr;
        } else {
            return nodoka_negInfStr;
        }
    }

    uint64_t S;
    int16_t N;
    uint16_t K;
    bool neg = value < 0;
    sepFloatNum(neg ? -value : value, &S, &N, &K);

    uint16_t *str;
    size_t len;
    if (K <= N && N <= 21) {
        len = N + neg;
        str = malloc(len * sizeof(uint16_t));
        for (int i = K - 1; i >= 0; i--) {
            str[neg + i] = '0' + div64(&S, 10);
        }
        for (int i = 0; i < N - K; i++) {
            str[neg + K + i] = '0';
        }
    } else if (0 < N && N <= 21) {
        len = K + 1 + neg;
        str = malloc(len * sizeof(uint16_t));
        str[neg + N] = '.';
        for (int i = K - 1; i >= N; i--) {
            str[neg + i + 1] = '0' + div64(&S, 10);
        }
        for (int i = N - 1; i >= 0; i--) {
            str[neg + i] = '0' + div64(&S, 10);
        }
    } else if (-6 < N && N <= 0) {
        len = N + K + 2 + neg;
        str = malloc(len * sizeof(uint16_t));
        str[neg] = '0';
        str[neg + 1] = '.';
        for (int i = 0; i < N; i++) {
            str[neg + i + 2] = '0';
        }
        for (int i = K - 1; i >= 0; i--) {
            str[neg + N + i + 2] = '0' + div64(&S, 10);
        }
    } else if (K == 1) {
        bool nNeg = N - 1 < 0;
        N = nNeg ? -N + 1 : N - 1;
        int expLen = countLen(N);
        len = expLen + neg + 3;
        str = malloc(len * sizeof(uint16_t));
        str[neg] = '0' + (uint16_t)S;
        str[neg + 1] = 'e';
        str[neg + 2] = nNeg ? '-' : '+';
        for (int i = expLen - 1; i >= 0; i--) {
            str[neg + i + 3] = '0' + N % 10;
            N /= 10;
        }
    } else {
        bool nNeg = N - 1 < 0;
        N = nNeg ? -N + 1 : N - 1;
        int expLen = countLen(N);
        len = expLen + neg + K + 3;
        str = malloc(len * sizeof(uint16_t));
        str[neg + 1] = '.';
        for (int i = K - 1; i >= 1; i--) {
            str[neg + i + 1] = '0' + div64(&S, 10);
        }
        str[neg] = '0' + (uint16_t)S;
        str[neg + K + 1] = 'e';
        str[neg + K + 2] = nNeg ? '-' : '+';
        for (int i = expLen - 1; i >= 0; i--) {
            str[neg + K + i + 3] = '0' + N % 10;
            N /= 10;
        }
    }
    if (neg)
        str[0] = '-';
    return nodoka_new_string((utf16_string_t) {
        .str = str, .len = len
    });
}
