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

static void blgy_prxy_bio_dump_work(struct work_struct *work)
{
    struct blgy_prxy_bio *bio =
        container_of(work, struct blgy_prxy_bio, work);

    BLGY_PRXY_DBG("%s: traced bio (id: %u, start_ts: %lli, end_ts: %lli, sector: %lli, size: %u, op: %u, cpu: %d);\n",
                  bio->dev->bdev->bd_disk->disk_name,
                  bio->info.id, bio->info.start_ts,
                  bio->info.end_ts, bio->info.sector,
                  bio->info.size, bio->info.op, bio->info.cpu);

    blgy_prxy_dump(bio->dump, bio);

    bio_endio(bio->bio);
    atomic_dec(&bio->dev->bio_inflight);

    blgy_prxy_bio_destroy(bio);
}

static void blgy_prxy_bio_dump_work_submit(struct blgy_prxy_bio *bio)
{
    int cpu = bio->info.cpu;
    bio->dump = blgy_prxy_dump_get_by_cpu(bio->dev->dumps, cpu);
    INIT_WORK(&bio->work, blgy_prxy_bio_dump_work);
    queue_work_on(cpu, bio->dev->dumps->wq, &bio->work);
}

static void blgy_prxy_bio_end_io(struct bio *clone)
{
    struct blgy_prxy_bio *bp_bio = clone->bi_private;
    struct bio *bio = bp_bio->bio;

    bp_bio->info.end_ts = ktime_get_boottime();
    bp_bio->info.status = clone->bi_status;

    bio->bi_status = clone->bi_status;
    bio_put(clone);

    blgy_prxy_bio_dump_work_submit(bp_bio);
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

    bp_bio->info.sector = bio->bi_iter.bi_sector;
    bp_bio->info.size = bio->bi_iter.bi_size;
    bp_bio->info.op = bio->bi_opf;
    bp_bio->info.cpu = smp_processor_id();
    bp_bio->info.start_ts = ktime_get_boottime() - dev->start_ts;
    bp_bio->info.payload.data = bio;
    if (bio_op(bio) == REQ_OP_WRITE)
        bp_bio->info.payload.size = bio->bi_iter.bi_size;
    else
        bp_bio->info.payload.size = 0;

    clone = bio_alloc_clone(file_bdev(dev->target), bio, GFP_NOIO, &fs_bio_set);
    if (!clone) {
        BLGY_PRXY_ERR("failed to allocate memory for bio clone");
        ret = -ENOMEM;
        goto err;
    }

    clone->bi_private = bp_bio;
    clone->bi_end_io = blgy_prxy_bio_end_io;

    bio_advance(bio, clone->bi_iter.bi_size);

    atomic_inc(&dev->bio_inflight);
    submit_bio_noacct(clone);

    return;

err:
    blgy_prxy_bio_destroy(bp_bio);
    bio->bi_status = ret;
    bio_endio(bio);
}
