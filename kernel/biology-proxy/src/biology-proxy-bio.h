#ifndef BLGY_PRXY_BIO_H_
#define BLGY_PRXY_BIO_H_

#include <linux/blk_types.h>
#include <linux/types.h>
#include <linux/workqueue_types.h>

struct blgy_prxy_dev;

struct blgy_prxy_bio_info {
    int cpu;
    uint32_t id;
    ktime_t start_ts;
    ktime_t end_ts;
    sector_t sector;
    unsigned int size;
    blk_opf_t op;
};

struct blgy_prxy_bio {
    struct bio *bio;
    struct blgy_prxy_bio_info info;
    struct blgy_prxy_dev *dev;
    struct work_struct dump_work;
};

void blgy_prxy_process_bio(struct blgy_prxy_dev *dev,
                                   struct bio *bio);

#endif
