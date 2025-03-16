#include "biology-proxy-dev.h"

#include <linux/blkdev.h>

#include "biology-proxy-bio.h"
#include "biology-proxy-common.h"

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
        BIOLOGY_PROXY_ERR("failed to allocate memory for biology_proxy_dev structure");
        goto err;
    }

    dev->state.target = bdev_file_open_by_path(cfg.target_path,
                                               FMODE_READ|FMODE_WRITE,
                                               THIS_MODULE, NULL);
    if (IS_ERR(dev->state.target)) {
        ret = PTR_ERR(dev->state.target);
        BIOLOGY_PROXY_ERR("failed to open bdev: %s, err: %i", cfg.target_path, ret);
        goto err_free_dev;
    }

    queue_limits_stack_bdev(&lims, file_bdev(dev->state.target), 0, "biology-proxy");

    dev->gd = blk_alloc_disk(&lims, NUMA_NO_NODE);
    if (IS_ERR(dev->gd)) {
        ret = PTR_ERR(dev->gd);
        BIOLOGY_PROXY_ERR("failed to alloc disk, err: %i", ret);
        goto err_put_file;
    }

    dev->gd->queue->queuedata = dev;
    dev->gd->major = 0;
    dev->gd->first_minor = 0;
    dev->gd->flags = GENHD_FL_NO_PART;
    dev->gd->fops = &biology_proxy_dev_ops;
    set_capacity(dev->gd, bdev_nr_sectors(file_bdev(dev->state.target)));

    blk_queue_flag_set(QUEUE_FLAG_NOMERGES, dev->gd->queue);
    blk_queue_flag_set(QUEUE_FLAG_NOXMERGES, dev->gd->queue);

    snprintf(dev->gd->disk_name, BDEVNAME_SIZE, "%s-bp",
             file_bdev(dev->state.target)->bd_disk->disk_name);

    ret = add_disk(dev->gd);
    if (ret) {
        BIOLOGY_PROXY_ERR("failed to add disk, err: %i", ret);
        goto err_put_disk;
    }

    dev->bdev = dev->gd->part0;
    biology_proxy_dev_state_init(&dev->state);

    biology_proxy_devs_add(dev);

    BIOLOGY_PROXY_INFO("created proxy device %s (target = %s)",
                       dev->gd->disk_name, cfg.target_path);

    return 0;

err_put_disk:
    put_disk(dev->gd);
err_put_file:
    bdev_fput(dev->state.target);
err_free_dev:
    kfree(dev);
err:
    return ret;
}

static void _biology_proxy_dev_destroy(struct biology_proxy_dev *dev)
{
    BIOLOGY_PROXY_INFO("removed proxy device %s", dev->gd->disk_name);
    del_gendisk(dev->gd);
    put_disk(dev->gd);
    bdev_fput(dev->state.target);
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
