#ifndef BLGY_PRXY_BIO_H_
#define BLGY_PRXY_BIO_H_

#include <linux/blk_types.h>
#include <linux/types.h>
#include <linux/workqueue_types.h>

struct blgy_prxy_dev;

struct blgy_prxy_bio_info_payload {
    size_t size;
    struct bio *data;
};

struct blgy_prxy_bio_info {
    int cpu;
    uint32_t id;
    s64 start_ts;
    s64 end_ts;
    sector_t sector;
    unsigned int size;
    blk_opf_t op;
    blk_status_t status;
    struct blgy_prxy_bio_info_payload payload;
};

struct blgy_prxy_bio {
    struct bio *bio;
    struct blgy_prxy_bio_info info;
    struct blgy_prxy_dev *dev;
    struct blgy_prxy_dump *dump;
    struct work_struct work;
};

void blgy_prxy_process_bio(struct blgy_prxy_dev *dev,
                                   struct bio *bio);

#endif
