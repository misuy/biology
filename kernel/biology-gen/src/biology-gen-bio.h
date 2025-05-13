#ifndef BLGY_GEN_BIO_H_
#define BLGY_GEN_BIO_H_

#include <linux/blk_types.h>
#include <linux/types.h>

struct blgy_prxy_bio_info_payload {
    size_t size;
    char *data;
};

struct blgy_prxy_bio_info {
    uint32_t id;
    int cpu;
    ktime_t start_ts;
    ktime_t end_ts;
    sector_t sector;
    unsigned int size;
    blk_opf_t op;
    blk_status_t status;
    struct blgy_prxy_bio_info_payload payload;
};

struct bio *
blgy_gen_bio_create(struct blgy_prxy_bio_info info, struct block_device *bdev);

#endif
