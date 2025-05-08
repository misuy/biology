#include "biology-gen-ctl.h"

#include <linux/module.h>

#include "biology-gen-common.h"
#include "biology-gen-worker.h"

static struct kobject ctl_kobject;

struct blgy_gen_sysfs_entry {
    struct attribute attr;
    ssize_t (*show) (char *);
    ssize_t (*store) (const char *, size_t);
};

static ssize_t blgy_gen_sysfs_entry_ctl_store(const char *buf, size_t cnt)
{
    int ret = 0;
    char *cpu;
    size_t cntc = cnt;
    int offset;
    struct blgy_gen_group_config config;

    offset = word_size(buf, cntc);
    if (offset < 0)
        return -EINVAL;

    if (strncmp(buf, "start", offset) != 0)
        return -EINVAL;

    buf += offset + 1;
    cntc -= offset + 1;

    if (parse_word((char **) &buf, &cntc, &config.name)) {
        ret = -EINVAL;
        goto err;
    }

    if (parse_word((char **) &buf, &cntc, &config.target_path)) {
        ret = -EINVAL;
        goto err_free_name;
    }

    if (parse_word((char **) &buf, &cntc, &config.dump_dir_path)) {
        ret = -EINVAL;
        goto err_free_target_path;
    }

    if (parse_word((char **) &buf, &cntc, &cpu)) {
        ret = -EINVAL;
        goto err_free_dump_dir_path;
    }

    if (kstrtoint(cpu, 10, &config.cpu_cnt)) {
        kfree(cpu);
        ret = -EINVAL;
        goto err_free_dump_dir_path;
    }

    kfree(cpu);

    if ((ret = blgy_gen_group_create(config))) {
        goto err_free_dump_dir_path;
    }

    kfree(config.dump_dir_path);
    kfree(config.target_path);

    return cnt;

err_free_dump_dir_path:
    kfree(config.dump_dir_path);
err_free_target_path:
    kfree(config.target_path);
err_free_name:
    kfree(config.name);
err:
    return ret;
}

static struct blgy_gen_sysfs_entry sysfs_entry_ctl = {
    .attr = { .name = "ctl", .mode = S_IWUSR },
    .store = blgy_gen_sysfs_entry_ctl_store,
};

static ssize_t blgy_gen_sysfs_attr_show(struct kobject *kobj,
                                        struct attribute *attr, char *page)
{
    struct blgy_gen_sysfs_entry *entry =
        container_of(attr, struct blgy_gen_sysfs_entry, attr);

    return entry->show(page);
}

static ssize_t blgy_gen_sysfs_attr_store(struct kobject *kobj,
                                         struct attribute *attr,
                                         const char *page, size_t cnt)
{
    struct blgy_gen_sysfs_entry *entry =
        container_of(attr, struct blgy_gen_sysfs_entry, attr);

    return entry->store(page, cnt);
}

static struct sysfs_ops blgy_gen_ops = {
    .show = blgy_gen_sysfs_attr_show,
    .store = blgy_gen_sysfs_attr_store,
};

static struct attribute *blgy_gen_attrs[] = {
    &sysfs_entry_ctl.attr,
    NULL,
};
ATTRIBUTE_GROUPS(blgy_gen);

static struct kobj_type ktype = {
    .sysfs_ops = &blgy_gen_ops,
    .default_groups = blgy_gen_groups,
};

int blgy_gen_ctl_init(void)
{
    int ret;
    ret = kobject_init_and_add(&ctl_kobject, &ktype,
                               &(((struct module *)(THIS_MODULE))->mkobj).kobj,
                               "module");

    if (ret) {
        BLGY_GEN_ERR("failed to init module sysfs kobject, err: %i", ret);
        return ret;
    }

    kobject_uevent(&ctl_kobject, KOBJ_ADD);
    return 0;
}

void blgy_gen_ctl_destroy(void)
{
    kobject_uevent(&ctl_kobject, KOBJ_REMOVE);
    kobject_del(&ctl_kobject);
    kobject_put(&ctl_kobject);
}
