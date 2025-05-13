#ifndef BLGY_PRXY_DUMP_H_
#define BLGY_PRXY_DUMP_H_

#include <linux/threads.h>
#include <linux/vfs.h>
#include <linux/mutex.h>

struct blgy_prxy_bio;
struct blgy_prxy_bio_serial_schema_field;

struct blgy_prxy_dump_file {
    struct file *file;
    loff_t offset;
};

#define BLGY_PRXY_DUMP_INITIAL_BUF_SIZE (4096 * 16)

struct blgy_prxy_dump {
    int cpu;
    struct blgy_prxy_dump_file file;
    struct blgy_prxy_bio_serial_schema_field **schema;
    char *buf;
    size_t size;
    size_t offset;
    struct mutex lock;
};
# 00001000
static inline size_t blgy_prxy_dump_buf_avail(struct blgy_prxy_dump *dump)
{
    return dump->size - dump->offset;
}

static inline char * blgy_prxy_dump_buf_pos(struct blgy_prxy_dump *dump)
{
    return dump->buf + dump->offset;
}

struct blgy_prxy_dumps {
    int cpus_num;
    struct blgy_prxy_dump *dumps;
    struct workqueue_struct *wq;
};

static inline struct blgy_prxy_dump *
blgy_prxy_dump_get_by_cpu(struct blgy_prxy_dumps *dumps, int cpu)
{
    return dumps->dumps + cpu;
}

int blgy_prxy_dumps_init(struct blgy_prxy_dumps *dumps,
                         struct blgy_prxy_bio_serial_schema_field **schema,
                         const char *dir_path);
void blgy_prxy_dumps_destroy(struct blgy_prxy_dumps *dumps);
int blgy_prxy_dump(struct blgy_prxy_dump *dump, struct blgy_prxy_bio *bio);

#endif
