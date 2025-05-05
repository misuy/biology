#ifndef BIO_INFO_H_
#define BIO_INFO_H_

#include <stddef.h>
#include <inttypes.h>

struct blgy_prxy_bio_info {
    uint32_t id;
    long long start_ts;
    long long end_ts;
    unsigned long long sector;
    int cpu;
    unsigned int size;
};

#endif
