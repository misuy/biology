#include "biology-proxy-bio.h"

#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/memory.h>

#include "biology-proxy-common.h"

static inline void
blgy_prxy_bio_info_init(struct blgy_prxy_bio_info *info, uint32_t id)
{
    info->id = id;
    info->start_ts = 0;
    info->end_ts = 0;
    info->sector = 0;
    info->size = 0;
}

static struct blgy_prxy_bio *
blgy_prxy_bio_create(struct blgy_prxy_dev *dev, struct bio *bio)
{
    struct blgy_prxy_bio *bp_bio =
        kmalloc(sizeof(struct blgy_prxy_bio), GFP_KERNEL);

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

static void blgy_prxy_bio_end_io(struct bio *clone)
{
    struct blgy_prxy_bio *pb_bio = clone->bi_private;
    struct bio *bio = pb_bio->bio;

    pb_bio->info.end_ts = ktime_get_boottime();

    BLGY_PRXY_INFO("%s: traced bio (id: %u, start_ts: %lli, end_ts: %lli, sector: %lli, size: %u);\n",
                       pb_bio->dev->bdev->bd_disk->disk_name,
                       pb_bio->info.id, pb_bio->info.start_ts,
                       pb_bio->info.end_ts, pb_bio->info.sector,
                       pb_bio->info.size);

    bio->bi_status = clone->bi_status;
    bio_put(clone);
    bio_endio(bio);

    blgy_prxy_bio_destroy(pb_bio);
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

    clone = bio_alloc_clone(file_bdev(dev->state.target), bio, GFP_NOIO, &fs_bio_set);
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

    bp_bio->info.start_ts = ktime_get_boottime();

    submit_bio_noacct(clone);

    return;

err:
    blgy_prxy_bio_destroy(bp_bio);
    bio->bi_status = ret;
    bio_endio(bio);
}

define_blgy_prxy_bio_serial_schema_field(
    info_id, info.id,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    info_start_ts, info.start_ts,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    info_end_ts, info.end_ts,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    info_sector, info.sector,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    info_size, info.size,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default
);

static struct blgy_prxy_bio_serial_schema_field *blgy_prxy_bio_serial_schema[] = {
    declare_blgy_prxy_bio_serial_schema_field(info_id),
    declare_blgy_prxy_bio_serial_schema_field(info_start_ts),
    declare_blgy_prxy_bio_serial_schema_field(info_end_ts),
    declare_blgy_prxy_bio_serial_schema_field(info_sector),
    declare_blgy_prxy_bio_serial_schema_field(info_size),
    NULL
};

static size_t blgy_prxy_bio_serial_schema_size(struct blgy_prxy_bio *bio)
{
    size_t size = 0, offset = 0;
    while (blgy_prxy_bio_serial_schema[offset++] != NULL)
        size += blgy_prxy_bio_serial_schema[offset]->size(bio);
    return size;
}

int blgy_prxy_bio_serialize(struct blgy_prxy_bio *bio, void **buf_p)
{
    size_t offset = 0;
    void *buf = kmalloc(blgy_prxy_bio_serial_schema_size(bio), GFP_KERNEL);
    void *buf_ptr = buf;

    if (buf == NULL)
        return -ENOMEM;

    while (blgy_prxy_bio_serial_schema[offset++] != NULL) {
        blgy_prxy_bio_serial_schema[offset]->serialize(
            blgy_prxy_bio_serial_schema[offset], bio, buf_ptr
        );
        buf_ptr += blgy_prxy_bio_serial_schema[offset]->size(bio);
    }

    *buf_p = buf;

    return 0;
}
