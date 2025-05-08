#include "biology-proxy-bio.h"

#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/memory.h>

#include "biology-proxy-common.h"
#include "biology-proxy-dump.h"
#include "biology-proxy-dev.h"

static inline void
blgy_prxy_bio_info_init(struct blgy_prxy_bio_info *info, uint32_t id)
{
    info->id = id;
    info->start_ts = 0;
    info->end_ts = 0;
    info->sector = 0;
    info->size = 0;
    info->cpu = -1;
}

static struct blgy_prxy_bio *
blgy_prxy_bio_create(struct blgy_prxy_dev *dev, struct bio *bio)
{
    struct blgy_prxy_bio *bp_bio =
        kzalloc(sizeof(struct blgy_prxy_bio), GFP_KERNEL);

    if (!bp_bio) {
        BLGY_PRXY_ERR("failed to allocate memory for blgy_prxy_bio structure");
        return (struct blgy_prxy_bio *) ERR_PTR(-ENOMEM);
    }

    bp_bio->bio = bio;
    bp_bio->dev = dev;
    blgy_prxy_bio_info_init(&bp_bio->info, blgy_prxy_dev_next_bio_id(dev));

    return bp_bio;
}

static void blgy_prxy_bio_destroy(struct blgy_prxy_bio *bio)
{
    kfree(bio);
}

static void blgy_prxy_dump_bio_work(struct work_struct *work)
{
    struct blgy_prxy_bio *bp_bio =
        container_of(work, struct blgy_prxy_bio, dump_work);

    BLGY_PRXY_DBG("%s: traced bio (id: %u, start_ts: %lli, end_ts: %lli, sector: %lli, size: %u, op: %u);\n",
                    bp_bio->dev->bdev->bd_disk->disk_name,
                    bp_bio->info.id, bp_bio->info.start_ts,
                    bp_bio->info.end_ts, bp_bio->info.sector,
                    bp_bio->info.size, bp_bio->bio->bi_opf);

    blgy_prxy_dump(bp_bio->dev->dump, bp_bio);

    bio_endio(bp_bio->bio);
    blgy_prxy_bio_destroy(bp_bio);
}

static void blgy_prxy_bio_end_io(struct bio *clone)
{
    struct blgy_prxy_bio *bp_bio = clone->bi_private;
    struct bio *bio = bp_bio->bio;

    bp_bio->info.op = clone->bi_opf;
    bp_bio->info.end_ts = ktime_get_boottime();

    bio->bi_status = clone->bi_status;
    bio_put(clone);

    queue_work(bp_bio->dev->dump->wq, &bp_bio->dump_work);
}

void blgy_prxy_process_bio(struct blgy_prxy_dev *dev, struct bio *bio)
{
    int ret = 0;
    struct bio *clone = NULL;
    struct blgy_prxy_bio *bp_bio = NULL;

    bp_bio = blgy_prxy_bio_create(dev, bio);
    if (IS_ERR(bp_bio)) {
        ret = PTR_ERR(bp_bio);
        goto err;
    }

    clone = bio_alloc_clone(file_bdev(dev->target), bio, GFP_NOIO, &fs_bio_set);
    if (!clone) {
        BLGY_PRXY_ERR("failed to allocate memory for bio clone");
        ret = -ENOMEM;
        goto err;
    }

    clone->bi_private = bp_bio;
    clone->bi_end_io = blgy_prxy_bio_end_io;

    bp_bio->info.sector = clone->bi_iter.bi_sector;
    bp_bio->info.size = bio->bi_iter.bi_size;
    bio_advance(bio, clone->bi_iter.bi_size);

    bp_bio->info.cpu = smp_processor_id();
    INIT_WORK(&bp_bio->dump_work, blgy_prxy_dump_bio_work);

    bp_bio->info.start_ts = ktime_get_boottime() - dev->start_ts;

    submit_bio_noacct(clone);

    return;

err:
    blgy_prxy_bio_destroy(bp_bio);
    bio->bi_status = ret;
    bio_endio(bio);
}
