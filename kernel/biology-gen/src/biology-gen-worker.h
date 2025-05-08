#ifndef BLGY_GEN_WORKER_H_
#define BLGY_GEN_WORKER_H_

#include <linux/kobject.h>
#include <linux/workqueue.h>

#include "biology-gen-parser.h"

struct blgy_gen_group_config {
    char *dump_dir_path;
    char *target_path;
    char *name;
    int cpu_cnt;
};

struct blgy_gen_group {
    const char *name;
    bool active;
    struct kref refcount;
    bool kobj_inited;
    struct kobject kobj;
};

struct blgy_gen_worker {
    struct blgy_gen_group *group;
    int cpu;
    ktime_t start_ts;
    struct blgy_gen_parser parser;
    struct file *target;
};

struct blgy_gen_work {
    struct blgy_gen_worker *worker;
    struct delayed_work work;
    struct bio *bio;
};

int blgy_gen_group_create(struct blgy_gen_group_config cfg);
void blgy_gen_group_destroy(struct kref *ref);
void blgy_gen_group_stop(struct blgy_gen_group *group);

int blgy_gen_groups_init(void);
void blgy_gen_groups_destroy(void);

#endif
