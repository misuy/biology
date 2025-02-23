#include "biology-proxy-dev.h"

#include <linux/blkdev.h>

#include "biology-proxy-bio.h"

struct biology_proxy_devs {
    struct list_head list;
    struct mutex lock;
};

struct biology_proxy_devs devs;

static void biology_proxy_devs_add(struct biology_proxy_dev *dev)
{
    mutex_lock(&devs.lock);
    list_add(&dev->list_node, &devs.list);
    mutex_unlock(&devs.lock);
}

static void biology_proxy_devs_remove(struct biology_proxy_dev *dev)
{
    mutex_lock(&devs.lock);
    list_del(&dev->list_node);
    mutex_unlock(&devs.lock);
}

static inline void
biology_proxy_dev_state_init(struct biology_proxy_dev_state *state)
{
    atomic_set(&state->bio_id_counter, -1);
}

static void biology_proxy_dev_submit_bio(struct bio *bio)
{
    struct biology_proxy_dev *dev = bio->bi_bdev->bd_queue->queuedata;

    printk("submit_bio begin\n");

    biology_proxy_process_bio(dev, bio);
}

struct block_device_operations biology_proxy_dev_ops = {
    .submit_bio = biology_proxy_dev_submit_bio,
};

int biology_proxy_dev_create(struct biology_proxy_dev_config cfg)
{
    int ret = 0;
    struct queue_limits lims = { 0 };
    struct biology_proxy_dev *dev =
        kmalloc(sizeof(struct biology_proxy_dev), GFP_KERNEL);

    if (!dev) {
        ret = -ENOMEM;
        goto err;
    }

    queue_limits_stack_bdev(&lims, cfg.target, 0, "biology-proxy");

    dev->gd = blk_alloc_disk(&lims, NUMA_NO_NODE);
    if (IS_ERR(dev->gd)) {
        ret = PTR_ERR(dev->gd);
        goto err_free_dev;
    }

    dev->cfg = cfg;

    dev->gd->queue->queuedata = dev;
    dev->gd->major = 0;
    dev->gd->first_minor = 0;
    dev->gd->flags = GENHD_FL_NO_PART;
    dev->gd->fops = &biology_proxy_dev_ops;
    set_capacity(dev->gd, bdev_nr_sectors(cfg.target));

    blk_queue_flag_set(QUEUE_FLAG_NOMERGES, dev->gd->queue);
    blk_queue_flag_set(QUEUE_FLAG_NOXMERGES, dev->gd->queue);

    snprintf(dev->gd->disk_name, BDEVNAME_SIZE, "%s-bp",
             cfg.target->bd_disk->disk_name);

    ret = add_disk(dev->gd);
    if (ret) {
        goto err_del_gd;
    }

    dev->bdev = dev->gd->part0;
    biology_proxy_dev_state_init(&dev->state);

    biology_proxy_devs_add(dev);

    return 0;

err_del_gd:
    del_gendisk(dev->gd);
err_free_dev:
    kfree(dev);
err:
    return ret;
}

static void _biology_proxy_dev_destroy(struct biology_proxy_dev *dev)
{
    del_gendisk(dev->gd);
    put_disk(dev->gd);
    kfree(dev);
}

void biology_proxy_dev_destroy(struct biology_proxy_dev *dev)
{
    biology_proxy_devs_remove(dev);
    _biology_proxy_dev_destroy(dev);
}

int biology_proxy_devs_init(void)
{
    mutex_init(&devs.lock);
    INIT_LIST_HEAD(&devs.list);
    return 0;
}

void biology_proxy_devs_destroy(void)
{
    struct biology_proxy_dev *pos = NULL, *n = NULL;
    mutex_lock(&devs.lock);
    list_for_each_entry_safe(pos, n, &devs.list, list_node) {
        list_del(&pos->list_node);
        _biology_proxy_dev_destroy(pos);
    }
    mutex_unlock(&devs.lock);
}
