#include "biology-proxy-bio.h"

#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/memory.h>

#include "biology-proxy-common.h"

static inline void
biology_proxy_bio_info_init(struct biology_proxy_bio_info *info, uint32_t id)
{
    info->id = id;
    info->start_ts = 0;
    info->end_ts = 0;
    info->sector = 0;
    info->size = 0;
}

static struct biology_proxy_bio *
biology_proxy_bio_create(struct biology_proxy_dev *dev, struct bio *bio)
{
    struct biology_proxy_bio *bp_bio =
        kmalloc(sizeof(struct biology_proxy_bio), GFP_KERNEL);

    if (!bp_bio) {
        BIOLOGY_PROXY_ERR("failed to allocate memory for biology_proxy_bio structure");
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

    pb_bio->info.end_ts = ktime_get_boottime();

    BIOLOGY_PROXY_INFO("%s: traced bio (id: %u, start_ts: %lli, end_ts: %lli, sector: %lli, size: %u);\n",
                       pb_bio->dev->bdev->bd_disk->disk_name,
                       pb_bio->info.id, pb_bio->info.start_ts,
                       pb_bio->info.end_ts, pb_bio->info.sector,
                       pb_bio->info.size);

    bio->bi_status = clone->bi_status;
    bio_put(clone);
    bio_endio(bio);

    biology_proxy_bio_destroy(pb_bio);
}

void biology_proxy_process_bio(struct biology_proxy_dev *dev, struct bio *bio)
{
    int ret = 0;
    struct bio *clone = NULL;
    struct biology_proxy_bio *bp_bio = NULL;

    bp_bio = biology_proxy_bio_create(dev, bio);
    if (IS_ERR(bp_bio)) {
        ret = PTR_ERR(bp_bio);
        goto err;
    }

    clone = bio_alloc_clone(file_bdev(dev->state.target), bio, GFP_NOIO, &fs_bio_set);
    if (!clone) {
        BIOLOGY_PROXY_ERR("failed to allocate memory for bio clone");
        ret = -ENOMEM;
        goto err;
    }

    clone->bi_private = bp_bio;
    clone->bi_end_io = biology_proxy_bio_end_io;

    bp_bio->info.sector = clone->bi_iter.bi_sector;
    bp_bio->info.size = bio->bi_iter.bi_size;
    bio_advance(bio, clone->bi_iter.bi_size);

    bp_bio->info.start_ts = ktime_get_boottime();

    submit_bio_noacct(clone);

    return;

err:
    biology_proxy_bio_destroy(bp_bio);
    bio->bi_status = ret;
    bio_endio(bio);
}
