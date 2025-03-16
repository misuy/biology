#ifndef BIOLOGY_PROXY_BIO_H_
#define BIOLOGY_PROXY_BIO_H_

#include <linux/types.h>

#include "biology-proxy-dev.h"

struct biology_proxy_bio_info {
    uint32_t id;
    ktime_t start_ts;
    ktime_t end_ts;
    sector_t sector;
    unsigned int size;
};

struct biology_proxy_bio {
    struct bio *bio;
    struct biology_proxy_bio_info info;
    struct biology_proxy_dev *dev;
};

void biology_proxy_process_bio(struct biology_proxy_dev *dev,
                                   struct bio *bio);

#endif
