#ifndef BIO_INFO_H_
#define BIO_INFO_H_

#include <stddef.h>
#include <inttypes.h>

struct blgy_prxy_bio_info_payload {
    size_t size;
    char *data;
};

struct blgy_prxy_bio_info {
    int cpu;
    uint32_t id;
    long long start_ts;
    long long end_ts;
    unsigned long long sector;
    unsigned int size;
    int op;
    unsigned char status;
    struct blgy_prxy_bio_info_payload payload;
};

#endif
