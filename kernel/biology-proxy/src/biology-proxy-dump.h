#ifndef BLGY_PRXY_DUMP_H_
#define BLGY_PRXY_DUMP_H_

#include <linux/threads.h>
#include <linux/vfs.h>
#include <linux/mutex.h>

struct blgy_prxy_bio;

struct blgy_prxy_dump_file {
    struct file *file;
    struct mutex lock;
    loff_t offset;
};

struct blgy_prxy_dump {
    int cpus_num;
    struct blgy_prxy_dump_file *files;
    struct workqueue_struct *wq;
};

int blgy_prxy_dump_init(struct blgy_prxy_dump *dump, const char *dir_path);
void blgy_prxy_dump_destroy(struct blgy_prxy_dump *dump);
int blgy_prxy_dump(struct blgy_prxy_dump *dump, struct blgy_prxy_bio *bio);

#endif
