#include "biology-proxy-dev-ctl.h"

#include <linux/blkdev.h>

#include "biology-proxy-dev.h"
#include "biology-proxy-common.h"

struct blgy_prxy_dev_ctl_sysfs_entry {
    struct attribute attr;
    ssize_t (*show) (struct blgy_prxy_dev *, char *);
    ssize_t (*store) (struct blgy_prxy_dev *, const char *, size_t);
};

static ssize_t _dev_ctl_enable(struct blgy_prxy_dev *dev, const char *buf,
                               ssize_t cnt)
{
    int ret;
    int offset;
    char *dump_payload;
    struct blgy_prxy_dev_enable_config config;

    offset = first_word_size(buf, cnt);

    if (offset == cnt)
        return -EINVAL;

    dump_payload = kzalloc(offset + 1, GFP_KERNEL);
    strncpy(dump_payload, buf, offset);
    config.dump_payload = strcmp("1", dump_payload) == 0;
    kfree(dump_payload);

    config.dump_dir = buf + offset + 1;
    if ((ret = blgy_prxy_dev_enable(dev, config)) != 0) {
        BLGY_PRXY_ERR("failed to enable biology proxy device, err: %i", ret);
        return ret;
    }

    return cnt;
}

static void _dev_ctl_disable(struct blgy_prxy_dev *dev)
{
    blgy_prxy_dev_disable(dev);
}

static void _dev_ctl_destroy(struct blgy_prxy_dev *dev)
{
    blgy_prxy_dev_disable(dev);
    blgy_prxy_dev_destroy(dev);
}

static ssize_t dev_ctl_sysfs_entry_ctl_store(struct blgy_prxy_dev *dev,
                                             const char *buf, size_t cnt)
{
    int ret = 0;
    int offset = first_word_size(buf, cnt);

    if (strncmp("enable", buf, offset) == 0) {
        if (offset == cnt)
            return -EINVAL;

        ret = _dev_ctl_enable(dev, buf + offset + 1, cnt - offset - 1);
        if (ret < 0)
            return ret;

        return cnt;
    }
    else if (strncmp("disable", buf, offset) == 0) {
        _dev_ctl_disable(dev);
        return cnt;
    }
    else if (strncmp("destroy", buf, offset) == 0) {
        _dev_ctl_destroy(dev);
        return cnt;
    }

    return -EINVAL;
}

static struct blgy_prxy_dev_ctl_sysfs_entry sysfs_entry_ctl = {
    .attr = { .name = "ctl", .mode = S_IWUSR },
    .store = dev_ctl_sysfs_entry_ctl_store,
};

static ssize_t dev_ctl_sysfs_entry_active_show(struct blgy_prxy_dev *dev,
                                               char *buf)
{
    return snprintf(buf, sizeof(int), "%d", dev->enabled);
}

static struct blgy_prxy_dev_ctl_sysfs_entry sysfs_entry_active = {
    .attr = { .name = "active", .mode = S_IRUSR },
    .show = dev_ctl_sysfs_entry_active_show,
};

static ssize_t dev_ctl_sysfs_entry_traced_show(struct blgy_prxy_dev *dev,
                                               char *buf)
{
    return snprintf(buf, sizeof(int), "%d",
                    atomic_read(&dev->bio_id_counter) + 1);
}

static struct blgy_prxy_dev_ctl_sysfs_entry sysfs_entry_traced = {
    .attr = { .name = "traced", .mode = S_IRUSR },
    .show = dev_ctl_sysfs_entry_traced_show,
};

static ssize_t dev_ctl_sysfs_entry_inflight_show(struct blgy_prxy_dev *dev,
                                               char *buf)
{
    return snprintf(buf, sizeof(int), "%d", atomic_read(&dev->bio_inflight));
}

static struct blgy_prxy_dev_ctl_sysfs_entry sysfs_entry_inflight = {
    .attr = { .name = "inflight", .mode = S_IRUSR },
    .show = dev_ctl_sysfs_entry_inflight_show,
};

static ssize_t dev_ctl_sysfs_attr_show(struct kobject *kobj,
                                       struct attribute *attr, char *page)
{
    struct blgy_prxy_dev_ctl_sysfs_entry *entry =
        container_of(attr, struct blgy_prxy_dev_ctl_sysfs_entry, attr);

    struct blgy_prxy_dev *dev = container_of(kobj, struct blgy_prxy_dev, kobj);

    return entry->show(dev, page);
}

static ssize_t dev_ctl_sysfs_attr_store(struct kobject *kobj,
                                        struct attribute *attr,
                                        const char *page, size_t cnt)
{
    struct blgy_prxy_dev_ctl_sysfs_entry *entry =
        container_of(attr, struct blgy_prxy_dev_ctl_sysfs_entry, attr);

    struct blgy_prxy_dev *dev = container_of(kobj, struct blgy_prxy_dev, kobj);

    return entry->store(dev, page, cnt);
}

static struct sysfs_ops dev_ctl_sysfs_ops = {
    .show = dev_ctl_sysfs_attr_show,
    .store = dev_ctl_sysfs_attr_store,
};

static struct attribute *dev_ctl_attrs[] = {
    &sysfs_entry_ctl.attr,
    &sysfs_entry_active.attr,
    &sysfs_entry_traced.attr,
    &sysfs_entry_inflight.attr,
    NULL,
};
ATTRIBUTE_GROUPS(dev_ctl);

static struct kobj_type ktype = {
    .sysfs_ops = &dev_ctl_sysfs_ops,
    .default_groups = dev_ctl_groups,
};

int blgy_prxy_dev_ctl_init(struct blgy_prxy_dev *dev)
{
    int ret;
    ret = kobject_init_and_add(&dev->kobj, &ktype,
                               &(((struct module *)(THIS_MODULE))->mkobj).kobj,
                               dev->bdev->bd_disk->disk_name);

    if (ret) {
        BLGY_PRXY_ERR("failed to init device sysfs ctl kobject, err: %i", ret);
        return ret;
    }

    kobject_uevent(&dev->kobj, KOBJ_ADD);
    return 0;
}

void blgy_prxy_dev_ctl_destroy(struct blgy_prxy_dev *dev)
{
    kobject_uevent(&dev->kobj, KOBJ_REMOVE);
    kobject_del(&dev->kobj);
    kobject_put(&dev->kobj);
}
