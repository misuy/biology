#include "biology-gen-worker.h"

#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/memory.h>

#include "biology-gen-bio.h"
#include "biology-gen-common.h"
#include "biology-gen-worker-ctl.h"
#include "biology-gen-parser.h"

atomic_t groups_cnt;

static int blgy_gen_work_submit(struct blgy_gen_work *work);

static int
blgy_gen_worker_create(struct blgy_gen_group *group, int cpu, char *dump_path,
                       const char *target_path)
{
    int ret = 0;
    struct blgy_gen_work *work;
    struct blgy_gen_worker *worker =
        kzalloc(sizeof(struct blgy_gen_worker), GFP_KERNEL);

    if (!worker) {
        ret = -ENOMEM;
        goto err;
    }

    worker->group = group;
    worker->cpu = cpu;
    worker->start_ts = ktime_get_boottime();

    kref_get(&worker->group->refcount);

    if ((ret = blgy_gen_parser_init(&worker->parser, dump_path))) {
        kfree(worker);
        goto err_free_worker;
    }

    worker->target = bdev_file_open_by_path(target_path,
                                            FMODE_READ|FMODE_WRITE,
                                            THIS_MODULE, NULL);

    if (IS_ERR(worker->target)) {
        ret = PTR_ERR(worker->target);
        BLGY_GEN_ERR("failed to open bdev: %s, err: %i", target_path, ret);
        goto err_parser_free;
    }

    work = kzalloc(sizeof(struct blgy_gen_work), GFP_KERNEL);
    work->worker = worker;
    blgy_gen_work_submit(work);

    return 0;

err_parser_free:
    blgy_gen_parser_free(&worker->parser);
err_free_worker:
    kref_put(&worker->group->refcount, blgy_gen_group_destroy);
    kfree(worker);
err:
    return ret;
}

static void blgy_gen_worker_destroy(struct blgy_gen_worker *worker)
{
    bdev_fput(worker->target);
    blgy_gen_parser_free(&worker->parser);
    kref_put(&worker->group->refcount, blgy_gen_group_destroy);
    kfree(worker);
}

static void blgy_gen_work_destroy(struct blgy_gen_work *work)
{
    blgy_gen_worker_destroy(work->worker);
    kfree(work);
}

static void blgy_gen_work(struct work_struct *_work)
{
    struct blgy_gen_work *work =
        container_of(to_delayed_work(_work), struct blgy_gen_work, work);

    BLGY_GEN_DBG("bio: sector %llu, size %u, io %u", work->bio->bi_iter.bi_sector, work->bio->bi_iter.bi_size, work->bio->bi_opf);

    submit_bio(work->bio);

    blgy_gen_work_submit(work);
}

static int blgy_gen_work_submit(struct blgy_gen_work *work)
{
    ktime_t now, delay;
    struct blgy_prxy_bio_info info;
    enum blgy_gen_parser_read_next_ret ret;

    if (!work->worker->group->active) {
        blgy_gen_work_destroy(work);
        return 0;
    }

    ret = blgy_gen_parser_read_next(&work->worker->parser, &info);
    if (ret == BLGY_GEN_PARSER_READ_NEXT_EMPTY) {
        blgy_gen_work_destroy(work);
        return 0;
    }
    else if (ret == BLGY_GEN_PARSER_READ_NEXT_ERR) {
        blgy_gen_work_destroy(work);
        return -EIO;
    }

    work->bio = blgy_gen_bio_create(info, file_bdev(work->worker->target));
    if (!work->bio) {
        blgy_gen_work_destroy(work);
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&work->work, blgy_gen_work);

    now = ktime_get_boottime();
    delay = ((now - work->worker->start_ts) < info.start_ts)
            ? info.start_ts - (now - work->worker->start_ts)
            : 0;

    BLGY_GEN_DBG("delay: %lld (ns) (now: %lld, start_ts: %lld)", delay, now - work->worker->start_ts, info.start_ts);

    schedule_delayed_work_on(work->worker->cpu, &work->work,
                             nsecs_to_jiffies(delay));

    return 0;
}

void blgy_gen_group_stop(struct blgy_gen_group *group)
{
    group->active = false;
}

int blgy_gen_group_create(struct blgy_gen_group_config cfg)
{
    int ret = 0, i;
    char *dump_file_path;
    struct blgy_gen_group *group =
        kzalloc(sizeof(struct blgy_gen_group), GFP_KERNEL);

    group->kobj_inited = false;

    if (!group) {
        BLGY_GEN_ERR("failed to alloc memory for group");
        ret = -ENOMEM;
        goto end;
    }

    group->name = cfg.name;
    group->active = true;
    kref_init(&group->refcount);

    for (i = 0; i < cfg.cpu_cnt; i++) {
        dump_file_path = kzalloc(PATH_MAX, GFP_KERNEL);
        sprintf(dump_file_path, "%s/dump%d", cfg.dump_dir_path, i);
        if ((ret = blgy_gen_worker_create(group, i, dump_file_path,
             cfg.target_path))) {
                goto end;
        }
        kfree(dump_file_path);
    }

    ret = blgy_gen_group_ctl_init(group);

end:
    kref_put(&group->refcount, blgy_gen_group_destroy);
    return ret;
}

void blgy_gen_group_destroy(struct kref *ref)
{
    struct blgy_gen_group *group =
        container_of(ref, struct blgy_gen_group, refcount);

    if (group->kobj_inited)
        blgy_gen_group_ctl_destroy(group);

    kfree(group->name);
    kfree(group);
}

int blgy_gen_groups_init(void)
{
    atomic_set(&groups_cnt, 0);
    return 0;
}

void blgy_gen_groups_destroy(void)
{
    while (atomic_read(&groups_cnt) != 0);
}
