#ifndef BIOLOGY_PROXY_DEV_H_
#define BIOLOGY_PROXY_DEV_H_

#include <linux/atomic.h>
#include <linux/types.h>

struct biology_proxy_dev_config {
    struct block_device *target;
};

struct biology_proxy_dev_state {
    atomic_t bio_id_counter;
};

struct biology_proxy_dev {
    struct block_device *bdev;
    struct gendisk *gd;
    struct biology_proxy_dev_config cfg;
    struct biology_proxy_dev_state state;
    struct list_head list_node;
};

static inline uint32_t
biology_proxy_dev_next_bio_id(struct biology_proxy_dev *dev)
{
    return atomic_inc_return(&dev->state.bio_id_counter);
}

int biology_proxy_devs_init(void);
void biology_proxy_devs_destroy(void);

int biology_proxy_dev_create(struct biology_proxy_dev_config cfg);
void biology_proxy_dev_destroy(struct biology_proxy_dev *dev);

#endif
