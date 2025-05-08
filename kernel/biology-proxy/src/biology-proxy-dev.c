#include "biology-proxy-dev.h"

#include <linux/blkdev.h>

#include "biology-proxy-bio.h"
#include "biology-proxy-common.h"
#include "biology-proxy-dev-ctl.h"
#include "biology-proxy-dump.h"

atomic_t devs_cnt;

static void blgy_prxy_dev_submit_bio(struct bio *bio)
{
    struct blgy_prxy_dev *dev = bio->bi_bdev->bd_queue->queuedata;

    if (dev->enabled) {
        blgy_prxy_process_bio(dev, bio);
    }
    else {
        bio_set_dev(bio, file_bdev(dev->target));
        submit_bio_noacct(bio);
    }
}

struct block_device_operations blgy_prxy_dev_ops = {
    .submit_bio = blgy_prxy_dev_submit_bio,
};

int blgy_prxy_dev_enable(struct blgy_prxy_dev *dev,
                         struct blgy_prxy_dev_enable_config config)
{
    int ret;

    if (dev->dump)
        return -EALREADY;

    dev->dump = kzalloc(sizeof(struct blgy_prxy_dump), GFP_KERNEL);
    if (!dev->dump)
        return -ENOMEM;

    if ((ret = blgy_prxy_dump_init(dev->dump, config.dump_dir))) {
        kfree(dev->dump);
        dev->dump = NULL;
        return ret;
    }

    atomic_set(&dev->bio_id_counter, -1);
    dev->start_ts = ktime_get_boottime();
    dev->enabled = true;

    BLGY_PRXY_INFO("proxy device %s enabled", dev->gd->disk_name);

    return 0;
}

void blgy_prxy_dev_disable(struct blgy_prxy_dev *dev)
{
    dev->enabled = false;

    if (!dev->dump)
        return;

    blgy_prxy_dump_destroy(dev->dump);
    kfree(dev->dump);
    dev->dump = NULL;

    BLGY_PRXY_INFO("proxy device %s disabled", dev->gd->disk_name);
}

int blgy_prxy_dev_create(struct blgy_prxy_dev_config cfg)
{
    int ret = 0;
    struct queue_limits lims = { 0 };
    struct blgy_prxy_dev *dev =
        kzalloc(sizeof(struct blgy_prxy_dev), GFP_KERNEL);

    if (!dev) {
        ret = -ENOMEM;
        BLGY_PRXY_ERR("failed to allocate memory for blgy_prxy_dev structure");
        goto err;
    }

    dev->target = bdev_file_open_by_path(cfg.target_path,
                                         FMODE_READ|FMODE_WRITE,
                                         THIS_MODULE, NULL);
    if (IS_ERR(dev->target)) {
        ret = PTR_ERR(dev->target);
        BLGY_PRXY_ERR("failed to open bdev: %s, err: %i", cfg.target_path, ret);
        goto err_free_dev;
    }

    queue_limits_stack_bdev(&lims, file_bdev(dev->target), 0, "biology-proxy");

    dev->gd = blk_alloc_disk(&lims, NUMA_NO_NODE);
    if (IS_ERR(dev->gd)) {
        ret = PTR_ERR(dev->gd);
        BLGY_PRXY_ERR("failed to alloc disk, err: %i", ret);
        goto err_put_file;
    }

    dev->gd->queue->queuedata = dev;
    dev->gd->major = 0;
    dev->gd->first_minor = 0;
    dev->gd->flags = GENHD_FL_NO_PART;
    dev->gd->fops = &blgy_prxy_dev_ops;
    set_capacity(dev->gd, bdev_nr_sectors(file_bdev(dev->target)));

    blk_queue_flag_set(QUEUE_FLAG_NOMERGES, dev->gd->queue);
    blk_queue_flag_set(QUEUE_FLAG_NOXMERGES, dev->gd->queue);

    snprintf(dev->gd->disk_name, BDEVNAME_SIZE, BLGY_PRXY_MOD_NAME_SHORT "%s",
             file_bdev(dev->target)->bd_disk->disk_name);

    ret = add_disk(dev->gd);
    if (ret) {
        BLGY_PRXY_ERR("failed to add disk, err: %i", ret);
        goto err_put_disk;
    }

    dev->bdev = dev->gd->part0;

    dev->enabled = false;
    dev->dump = NULL;

    ret = blgy_prxy_dev_ctl_init(dev);
    if (ret) {
        BLGY_PRXY_ERR("failed to init device sysfs, err: %i", ret);
        goto err_del_gendisk;
    }

    atomic_inc(&devs_cnt);

    BLGY_PRXY_INFO("created proxy device %s (target = %s)",
                       dev->gd->disk_name, cfg.target_path);

    return 0;

err_del_gendisk:
    del_gendisk(dev->gd);
err_put_disk:
    put_disk(dev->gd);
err_put_file:
    bdev_fput(dev->target);
err_free_dev:
    kfree(dev);
err:
    return ret;
}

#define BLGY_PRXY_DEV_DESTROY_WORK_DELAY_MS 300

struct blgy_prxy_dev_destroy_work {
    struct delayed_work work;
    struct blgy_prxy_dev *dev;
};

static void _blgy_prxy_dev_destroy_work(struct work_struct *work)
{
    struct blgy_prxy_dev_destroy_work *destroy_work =
        container_of(to_delayed_work(work), struct blgy_prxy_dev_destroy_work,
                     work);
    struct blgy_prxy_dev *dev = destroy_work->dev;

    BLGY_PRXY_INFO("destroying proxy device %s", dev->gd->disk_name);

    blgy_prxy_dev_ctl_destroy(dev);
    del_gendisk(dev->gd);
    put_disk(dev->gd);
    bdev_fput(dev->target);
    kfree(dev);
    kfree(destroy_work);
    atomic_dec(&devs_cnt);
}

void blgy_prxy_dev_destroy(struct blgy_prxy_dev *dev)
{
    struct blgy_prxy_dev_destroy_work *work =
        kzalloc(sizeof(struct blgy_prxy_dev_destroy_work), GFP_KERNEL);

    if (!work) {
        BLGY_PRXY_ERR("failed to allocate memory for dev %s destroy work", dev->gd->disk_name);
        return;
    }

    work->dev = dev;
    INIT_DELAYED_WORK(&work->work, _blgy_prxy_dev_destroy_work);
    queue_delayed_work(system_unbound_wq, &work->work,
                       msecs_to_jiffies(BLGY_PRXY_DEV_DESTROY_WORK_DELAY_MS));
}

int blgy_prxy_devs_init(void)
{
    atomic_set(&devs_cnt, 0);
    return 0;
}

void blgy_prxy_devs_destroy(void)
{
    while (atomic_read(&devs_cnt) != 0);
}
