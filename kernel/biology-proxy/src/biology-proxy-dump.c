#include "biology-proxy-dump.h"

#include <linux/bio.h>
#include <linux/types.h>
#include <linux/smp.h>

#include "biology-proxy-common.h"
#include "biology-proxy-bio.h"

static void blgy_prxy_dump_file_destroy(struct blgy_prxy_dump_file *file)
{
    if (!file->file)
        return;

    filp_close(file->file, 0);
    file->file = NULL;
}

void blgy_prxy_dump_destroy(struct blgy_prxy_dump *dump)
{
    int i;
    for (i = 0; i < dump->cpus_num; i++)
        blgy_prxy_dump_file_destroy(&dump->files[i]);

    if (dump->wq) {
        flush_workqueue(dump->wq);
        destroy_workqueue(dump->wq);
        dump->wq = NULL;
    }
}

static int blgy_prxy_dump_file_init(struct blgy_prxy_dump_file *file, char *path)
{
    BLGY_PRXY_DBG("opening dump file %s", path);
    mutex_init(&file->lock);
    file->offset = 0;
    file->file = filp_open(path, O_RDWR | O_CREAT | O_DSYNC, 0);
    if (!file->file) {
        BLGY_PRXY_ERR("failed to open dump file %s", path);
        return -EINVAL;
    }
    return 0;
}

static void _dump_build_file_path(char *path, const char *dir_path, int cpun)
{
    sprintf(path, "%s/dump%d", dir_path, cpun);
}

int blgy_prxy_dump_init(struct blgy_prxy_dump *dump, const char *dir_path)
{
    int i = 0, ret = 0;
    char *file_path = kzalloc(PATH_MAX, GFP_KERNEL);
    dump->cpus_num = num_online_cpus();
    dump->files =
        kzalloc(sizeof(struct blgy_prxy_dump_file) * dump->cpus_num, GFP_KERNEL);

    for (i = 0; i < dump->cpus_num; i++)
        dump->files[i].file = NULL;

    dump->wq = NULL;

    for (i = 0; i < dump->cpus_num; i++) {
        _dump_build_file_path(file_path, dir_path, i);
        if ((ret = blgy_prxy_dump_file_init(&dump->files[i], file_path)) != 0)
            goto out;
    }

    dump->wq = alloc_workqueue("dump_%s", 0, 0, dir_path);

out:
    if (ret)
        blgy_prxy_dump_destroy(dump);

    kfree(file_path);
    return ret;
}

int blgy_prxy_dump(struct blgy_prxy_dump *dump, struct blgy_prxy_bio *bio)
{
    int ret = 0;
    ssize_t size, written;
    char *buf;
    struct iov_iter iter;
    struct bio_vec bvec;
    int cpu;

    BLGY_PRXY_DBG("dumping bio %u", bio->info.id);

    if ((size = blgy_prxy_bio_serialize(bio, &buf)) < 0)
        return size;

    cpu = bio->info.cpu;

    bvec.bv_page = virt_to_page(buf);
    bvec.bv_offset = offset_in_page(buf);
    bvec.bv_len = size;

    iov_iter_bvec(&iter, WRITE, &bvec, 1, size);

    mutex_lock(&dump->files[cpu].lock);

    written = vfs_iter_write(dump->files[cpu].file, &iter,
                             &dump->files[cpu].offset, 0);

    if (size != written) {
        BLGY_PRXY_ERR("failed to dump bio %u (IO error)", bio->info.id);
        ret = -EIO;
        goto out;
    }

    vfs_fsync_range(dump->files[cpu].file, dump->files[cpu].offset - size,
                    dump->files[cpu].offset, 0);

out:
    mutex_unlock(&dump->files[cpu].lock);
    kfree(buf);
    return ret;
}
