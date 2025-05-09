#ifndef BLGY_PRXY_DEV_H_
#define BLGY_PRXY_DEV_H_

#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/kobject.h>

struct blgy_prxy_dev_enable_config {
    const char *dump_dir;
};

struct blgy_prxy_dev_config {
    const char *target_path;
};

struct blgy_prxy_dev {
    struct block_device *bdev;
    struct gendisk *gd;
    struct file *target;
    bool enabled;
    struct blgy_prxy_dumps *dumps;
    atomic_t bio_id_counter;
    ktime_t start_ts;
    struct kobject kobj;
};

static inline uint32_t
blgy_prxy_dev_next_bio_id(struct blgy_prxy_dev *dev)
{
    return atomic_inc_return(&dev->bio_id_counter);
}

int blgy_prxy_devs_init(void);
void blgy_prxy_devs_destroy(void);

int blgy_prxy_dev_create(struct blgy_prxy_dev_config cfg);
void blgy_prxy_dev_destroy(struct blgy_prxy_dev *dev);
int blgy_prxy_dev_enable(struct blgy_prxy_dev *dev,
                         struct blgy_prxy_dev_enable_config config);
void blgy_prxy_dev_disable(struct blgy_prxy_dev *dev);

#endif
