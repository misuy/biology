#include "biology-gen-worker-ctl.h"

#include <linux/blkdev.h>

#include "biology-gen-common.h"
#include "biology-gen-worker.h"

struct blgy_gen_group_sysfs_entry {
    struct attribute attr;
    ssize_t (*show) (struct blgy_gen_group *, char *);
    ssize_t (*store) (struct blgy_gen_group *, const char *, size_t);
};

static ssize_t blgy_gen_group_sysfs_entry_ctl_store(struct blgy_gen_group *group,
                                                    const char *buf, size_t cnt)
{
    if (strcmp(buf, "stop") != 0)
        return -EINVAL;

    blgy_gen_group_stop(group);

    return cnt;
}

static struct blgy_gen_group_sysfs_entry sysfs_entry_ctl = {
    .attr = { .name = "ctl", .mode = S_IWUSR },
    .store = blgy_gen_group_sysfs_entry_ctl_store,
};

static ssize_t blgy_gen_group_sysfs_attr_show(struct kobject *kobj,
                                              struct attribute *attr,
                                              char *page)
{
    struct blgy_gen_group_sysfs_entry *entry =
        container_of(attr, struct blgy_gen_group_sysfs_entry, attr);

    struct blgy_gen_group *group =
        container_of(kobj, struct blgy_gen_group, kobj);

    return entry->show(group, page);
}

static ssize_t blgy_gen_group_sysfs_attr_store(struct kobject *kobj,
                                               struct attribute *attr,
                                               const char *page, size_t cnt)
{
    struct blgy_gen_group_sysfs_entry *entry =
        container_of(attr, struct blgy_gen_group_sysfs_entry, attr);

    struct blgy_gen_group *group =
        container_of(kobj, struct blgy_gen_group, kobj);

    return entry->store(group, page, cnt);
}

static struct sysfs_ops blgy_gen_group_ops = {
    .show = blgy_gen_group_sysfs_attr_show,
    .store = blgy_gen_group_sysfs_attr_store,
};

static struct attribute *blgy_gen_group_attrs[] = {
    &sysfs_entry_ctl.attr,
    NULL,
};
ATTRIBUTE_GROUPS(blgy_gen_group);

static struct kobj_type ktype = {
    .sysfs_ops = &blgy_gen_group_ops,
    .default_groups = blgy_gen_group_groups,
};

int blgy_gen_group_ctl_init(struct blgy_gen_group *group)
{
    int ret;
    ret = kobject_init_and_add(&group->kobj, &ktype,
                               &(((struct module *)(THIS_MODULE))->mkobj).kobj,
                               group->name);

    if (ret) {
        BLGY_GEN_ERR("failed to init group sysfs kobject, err: %i", ret);
        return ret;
    }

    kobject_uevent(&group->kobj, KOBJ_ADD);
    group->kobj_inited = true;
    return 0;
}

void blgy_gen_group_ctl_destroy(struct blgy_gen_group *group)
{
    group->kobj_inited = false;
    kobject_uevent(&group->kobj, KOBJ_REMOVE);
    kobject_del(&group->kobj);
    kobject_put(&group->kobj);
}
