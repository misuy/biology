#ifndef BLGY_PRXY_DEV_H_
#define BLGY_PRXY_DEV_H_

#include <linux/atomic.h>
#include <linux/types.h>

struct blgy_prxy_dev_config {
    const char *target_path;
};

struct blgy_prxy_dev_state {
    struct file *target;
    atomic_t bio_id_counter;
};

struct blgy_prxy_dev {
    struct block_device *bdev;
    struct gendisk *gd;
    struct blgy_prxy_dev_state state;
    struct list_head list_node;
};

static inline uint32_t
blgy_prxy_dev_next_bio_id(struct blgy_prxy_dev *dev)
{
    return atomic_inc_return(&dev->state.bio_id_counter);
}

int blgy_prxy_devs_init(void);
void blgy_prxy_devs_destroy(void);

int blgy_prxy_dev_create(struct blgy_prxy_dev_config cfg);
void blgy_prxy_dev_destroy(struct blgy_prxy_dev *dev);

#endif
