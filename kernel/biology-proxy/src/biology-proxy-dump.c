#include "biology-proxy-dump.h"

#include <linux/bio.h>
#include <linux/types.h>
#include <linux/smp.h>

#include "biology-proxy-common.h"
#include "biology-proxy-bio.h"
#include "biology-proxy-bio-serial.h"

static void blgy_prxy_dump_file_destroy(struct blgy_prxy_dump_file *file)
{
    if (!file->file)
        return;

    filp_close(file->file, 0);
    file->file = NULL;
}

static int blgy_prxy_dump_file_init(struct blgy_prxy_dump_file *file, char *path)
{
    BLGY_PRXY_DBG("opening dump file %s", path);
    file->offset = 0;
    file->file = filp_open(path, O_RDWR | O_CREAT | O_DSYNC, 0);
    if (!file->file) {
        BLGY_PRXY_ERR("failed to open dump file %s", path);
        return -EINVAL;
    }
    return 0;
}

static int blgy_prxy_dump_file_write(struct blgy_prxy_dump_file *file,
                                     char *buf, size_t size)
{
    struct iov_iter iter;
    struct bio_vec bvec;
    ssize_t written;

    BLGY_PRXY_INFO("dump file write %lu", size);

    bvec.bv_page = virt_to_page(buf);
    bvec.bv_offset = offset_in_page(buf);
    bvec.bv_len = size;

    iov_iter_bvec(&iter, WRITE, &bvec, 1, size);

    written = vfs_iter_write(file->file, &iter, &file->offset, 0);

    if (size != written) {
        BLGY_PRXY_ERR("failed to write dump file (IO error)");
        return -EIO;
    }

    vfs_fsync_range(file->file, file->offset - size, file->offset, 0);

    return 0;
}

static int blgy_prxy_dump_schema(struct blgy_prxy_dump *dump)
{
    ssize_t size;

    if ((size =
         blgy_prxy_bio_serial_schema_serialize(dump->schema,
                                               blgy_prxy_dump_buf_pos(dump)))
        < 0)
        return size;

    dump->offset += size;

    return 0;
}

static int blgy_prxy_dump_buf_init(struct blgy_prxy_dump *dump)
{
    dump->buf = kmalloc(BLGY_PRXY_DUMP_INITIAL_BUF_SIZE, GFP_KERNEL);
    if (!dump->buf)
        return -ENOMEM;

    dump->size = BLGY_PRXY_DUMP_INITIAL_BUF_SIZE;
    dump->offset = 0;

    return 0;
}

static int blgy_prxy_dump_buf_drain(struct blgy_prxy_dump *dump)
{
    int ret;

    if (dump->offset == 0)
        return 0;

    ret = blgy_prxy_dump_file_write(&dump->file, dump->buf, dump->offset);
    dump->offset = 0;
    return ret;
}

static void blgy_prxy_dump_buf_destroy(struct blgy_prxy_dump *dump)
{
    blgy_prxy_dump_buf_drain(dump);
    kfree(dump->buf);
    dump->buf = NULL;
    dump->size = 0;
    dump->offset = 0;
}

static int blgy_prxy_dump_buf_resize(struct blgy_prxy_dump *dump, size_t size)
{
    dump->buf = krealloc(dump->buf, size, GFP_KERNEL);
    if (!dump->buf)
        return -ENOMEM;
    dump->size = size;
    return 0;
}

static int blgy_prxy_dump_buf_prepare(struct blgy_prxy_dump *dump, size_t size)
{
    int ret;

    if (blgy_prxy_dump_buf_avail(dump) >= size)
        return 0;

    if ((ret = blgy_prxy_dump_buf_drain(dump)))
        return ret;

    if (blgy_prxy_dump_buf_avail(dump) >= size)
        return 0;

    return blgy_prxy_dump_buf_resize(dump, size);
}

static void _dump_build_file_path(char *path, const char *dir_path, int cpun)
{
    sprintf(path, "%s/dump%d", dir_path, cpun);
}

static void blgy_prxy_dump_destroy(struct blgy_prxy_dump *dump)
{
    blgy_prxy_dump_buf_destroy(dump);
    blgy_prxy_dump_file_destroy(&dump->file);
}

static int
blgy_prxy_dump_init(struct blgy_prxy_dump *dump,
                    struct blgy_prxy_bio_serial_schema_field **schema,
                    const char *dir_path, int cpu)
{
    int ret = 0;
    char *file_path = kzalloc(PATH_MAX, GFP_KERNEL);
    if (!file_path)
        return -ENOMEM;

    dump->cpu = cpu;
    dump->schema = schema;
    mutex_init(&dump->lock);

    _dump_build_file_path(file_path, dir_path, dump->cpu);
    if ((ret = blgy_prxy_dump_file_init(&dump->file, file_path)))
        goto out;

    if ((ret = blgy_prxy_dump_buf_init(dump)))
        goto out;

    ret = blgy_prxy_dump_schema(dump);

out:
    if (ret)
        blgy_prxy_dump_destroy(dump);

    kfree(file_path);
    return ret;
}

int blgy_prxy_dumps_init(struct blgy_prxy_dumps *dumps,
                         struct blgy_prxy_bio_serial_schema_field **schema,
                         const char *dir_path)
{
    int cpu, ret;

    dumps->cpus_num = num_online_cpus();
    dumps->dumps =
        kzalloc(sizeof(struct blgy_prxy_dump) * dumps->cpus_num, GFP_KERNEL);

    if (!dumps->dumps)
        return -ENOMEM;

    for (cpu = 0; cpu < dumps->cpus_num; cpu++) {
        if ((ret = blgy_prxy_dump_init(blgy_prxy_dump_get_by_cpu(dumps, cpu),
                                       schema, dir_path, cpu))) {
            kfree(dumps->dumps);
            return ret;
        }
    }

    dumps->wq = alloc_workqueue("dump_%s", 0, 0, dir_path);

    return 0;
}

void blgy_prxy_dumps_destroy(struct blgy_prxy_dumps *dumps)
{
    int cpu;

    flush_workqueue(dumps->wq);
    destroy_workqueue(dumps->wq);

    for (cpu = 0; cpu < dumps->cpus_num; cpu++) {
        blgy_prxy_dump_destroy(blgy_prxy_dump_get_by_cpu(dumps, cpu));
    }

    kfree(dumps->dumps);
}

int blgy_prxy_dump(struct blgy_prxy_dump *dump, struct blgy_prxy_bio *bio)
{
    int ret = 0;
    ssize_t size;

    mutex_lock(&dump->lock);

    size = blgy_prxy_bio_serial_size(&bio->info, dump->schema) + sizeof(size_t);

    if ((ret = blgy_prxy_dump_buf_prepare(dump, size))) {
        BLGY_PRXY_ERR("failed to prepare dump buf");
        mutex_unlock(&dump->lock);
        return ret;
    }

    if ((size = blgy_prxy_bio_serialize(&bio->info,
                                        blgy_prxy_dump_buf_pos(dump),
                                        dump->schema)) < 0) {
        BLGY_PRXY_ERR("failed to serialize bio");
        mutex_unlock(&dump->lock);
        return size;
    }

    dump->offset += size;

    mutex_unlock(&dump->lock);

    return 0;
}
