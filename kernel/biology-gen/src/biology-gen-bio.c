#include "biology-gen-bio.h"

#include <linux/bio.h>

#include "biology-gen-common.h"

static void blgy_gen_bio_endio(struct bio *bio)
{
    BLGY_GEN_DBG("endio: %u", bio->bi_status);
    kfree(bio->bi_private);
    bio_put(bio);
}

struct bio *
blgy_gen_bio_create(struct blgy_prxy_bio_info info, struct block_device *bdev)
{
    char *buf;
    struct bio *bio = bio_alloc(bdev, 1, info.op, GFP_KERNEL);

    if (!bio) {
        BLGY_GEN_ERR("failed to allocate bio");
        return NULL;
    }

    buf = kmalloc(info.size, GFP_KERNEL);
    if (!buf) {
        BLGY_GEN_ERR("failed to allocate buffer for bio payload");
        return NULL;
    }

    bio->bi_iter.bi_sector = info.sector;
    if (bio_add_page(bio, virt_to_page(buf), info.size, offset_in_page(buf))
        != info.size) {
        BLGY_GEN_ERR("failed to add payload to bio");
        return NULL;
    }

    bio->bi_private = buf;
    bio->bi_end_io = blgy_gen_bio_endio;

    return bio;
}
