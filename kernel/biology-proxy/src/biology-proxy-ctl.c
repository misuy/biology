#include "biology-proxy-ctl.h"

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>

#include "biology-proxy-common.h"
#include "biology-proxy-dev.h"

static struct kobject ctl_kobject;

struct blgy_prxy_ctl_sysfs_entry {
    struct attribute attr;
    ssize_t (*show) (char *);
    ssize_t (*store) (const char *, size_t);
};

static ssize_t _ctl_create(const char *buf, ssize_t cnt)
{
    int ret = 0;
    struct blgy_prxy_dev_config config;

    if (cnt <= 0)
        return -EINVAL;

    config.target_path = buf;
    if ((ret = blgy_prxy_dev_create(config)) != 0) {
        BLGY_PRXY_ERR("failed to create biology proxy device, err: %i", ret);
        return ret;
    }

    return cnt;
}

static ssize_t ctl_sysfs_entry_ctl_store(const char *buf, size_t cnt)
{
    int ret = 0;
    int offset = first_word_size(buf, cnt);

    if (offset < 0)
        return -EINVAL;

    if (strncmp("create", buf, offset) == 0) {
        ret = _ctl_create(buf + offset + 2, cnt - offset - 2);
        if (ret < 0)
            return ret;

        return cnt;
    }

    return -EINVAL;
}

static struct blgy_prxy_ctl_sysfs_entry sysfs_entry_ctl = {
    .attr = { .name = "ctl", .mode = S_IWUSR },
    .store = ctl_sysfs_entry_ctl_store,
};


static ssize_t ctl_sysfs_attr_show(struct kobject *kobj, struct attribute *attr,
                                   char *page)
{
    struct blgy_prxy_ctl_sysfs_entry *entry =
        container_of(attr, struct blgy_prxy_ctl_sysfs_entry, attr);

    return entry->show(page);
}

static ssize_t ctl_sysfs_attr_store(struct kobject *kobj, struct attribute *attr,
                                    const char *page, size_t cnt)
{
    struct blgy_prxy_ctl_sysfs_entry *entry =
        container_of(attr, struct blgy_prxy_ctl_sysfs_entry, attr);

    return entry->store(page, cnt);
}

static struct sysfs_ops ctl_sysfs_ops = {
    .show = ctl_sysfs_attr_show,
    .store = ctl_sysfs_attr_store,
};


static struct attribute *ctl_attrs[] = {
    &sysfs_entry_ctl.attr,
    NULL,
};
ATTRIBUTE_GROUPS(ctl);


static struct kobj_type ktype = {
    .sysfs_ops = &ctl_sysfs_ops,
    .default_groups = ctl_groups,
};


int blgy_prxy_ctl_init(void)
{
    int ret;
    if ((ret = kobject_init_and_add(&ctl_kobject, &ktype,
                                    &(((struct module *)(THIS_MODULE))->mkobj).kobj,
                                    "module")))
    {
        BLGY_PRXY_ERR("failed to init sysfs ctl kobject, err: %i", ret);
        return ret;
    }

    kobject_uevent(&ctl_kobject, KOBJ_ADD);
    return 0;
}

void blgy_prxy_ctl_destroy(void)
{
    kobject_uevent(&ctl_kobject, KOBJ_REMOVE);
    kobject_del(&ctl_kobject);
    kobject_put(&ctl_kobject);
}
