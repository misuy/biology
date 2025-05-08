#ifndef BLGY_PRXY_DUMP_H_
#define BLGY_PRXY_DUMP_H_

#include <linux/threads.h>
#include <linux/vfs.h>
#include <linux/mutex.h>

struct blgy_prxy_bio;
struct blgy_prxy_bio_serial_schema_field;

struct blgy_prxy_dump_file {
    struct file *file;
    struct mutex lock;
    loff_t offset;
};

#define BLGY_PRXY_BUF_SIZE 4096

struct blgy_prxy_dump {
    int cpus_num;
    struct blgy_prxy_dump_file *files;
    struct blgy_prxy_bio_serial_schema_field **schema;
    char *buf;
    size_t size;
    size_t offset;
    struct workqueue_struct *wq;
};

int blgy_prxy_dump_init(struct blgy_prxy_dump *dump, const char *dir_path);
void blgy_prxy_dump_destroy(struct blgy_prxy_dump *dump);
int blgy_prxy_dump(struct blgy_prxy_dump *dump, struct blgy_prxy_bio *bio);

#endif
