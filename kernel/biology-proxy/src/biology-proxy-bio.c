#include "biology-proxy-bio.h"

#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/memory.h>

static inline void
biology_proxy_bio_info_init(struct biology_proxy_bio_info *info, uint32_t id)
{
    info->id = id;
    info->start_ts = 0;
    info->end_ts = 0;
}

static struct biology_proxy_bio *
biology_proxy_bio_create(struct biology_proxy_dev *dev, struct bio *bio)
{
    struct biology_proxy_bio *bp_bio =
        kmalloc(sizeof(struct biology_proxy_bio), GFP_KERNEL);

    if (!bp_bio) {
        return (struct biology_proxy_bio *) ERR_PTR(-ENOMEM);
    }

    bp_bio->bio = bio;
    bp_bio->dev = dev;
    biology_proxy_bio_info_init(&bp_bio->info, biology_proxy_dev_next_bio_id(dev));

    return bp_bio;
}

static void biology_proxy_bio_destroy(struct biology_proxy_bio *bio)
{
    kfree(bio);
}

static void biology_proxy_bio_end_io(struct bio *clone)
{
    struct biology_proxy_bio *pb_bio = clone->bi_private;
    struct bio *bio = pb_bio->bio;
    blk_status_t status = clone->bi_status;

    bio_put(clone);

    printk("bio_end_io begin\n");

    pb_bio->info.end_ts = ktime_get_boottime();

    bio->bi_status = status;
    bio_endio(bio);

    printk("biology_proxy: bio_end_io (id: %u, begin: %lli, end: %lli);\n",
           pb_bio->info.id, pb_bio->info.start_ts, pb_bio->info.end_ts);

    biology_proxy_bio_destroy(pb_bio);
}

void biology_proxy_process_bio(struct biology_proxy_dev *dev,
                                   struct bio *bio)
{
    int ret = 0;
    struct bio *clone = NULL;
    struct biology_proxy_bio *bp_bio = NULL;

    printk("bio_create\n");

    bp_bio = biology_proxy_bio_create(dev, bio);
    if (IS_ERR(bp_bio)) {
        ret = PTR_ERR(bp_bio);
        goto err;
    }

    clone = bio_alloc_clone(dev->cfg.target, bio, GFP_NOIO, &fs_bio_set);
    if (!clone) {
        ret = -ENOMEM;
        goto err;
    }

    clone->bi_private = bp_bio;
    clone->bi_end_io = biology_proxy_bio_end_io;

    bio_advance(bio, clone->bi_iter.bi_size);

    bp_bio->info.start_ts = ktime_get_boottime();

    submit_bio_noacct(clone);

    return;

err:
    biology_proxy_bio_destroy(bp_bio);
    bio->bi_status = ret;
    bio_endio(bio);
}
