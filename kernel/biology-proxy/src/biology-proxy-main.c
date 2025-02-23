#include <linux/blkdev.h>
#include <linux/module.h>

#include "biology-proxy-dev.h"

char *target = NULL;

struct file *file;

static int __init biology_proxy_init(void)
{
    int ret = 0;

    if (!target) {
        ret = -EINVAL;
        goto err;
    }

    ret = biology_proxy_devs_init();
    if (ret) {
        goto err;
    }

    file = bdev_file_open_by_path(target, FMODE_READ|FMODE_WRITE, THIS_MODULE, NULL);
    if (IS_ERR(file)) {
        ret = PTR_ERR(file);
        goto err_devs_destroy;
    }

    struct biology_proxy_dev_config config = { .target = file_bdev(file) };
    ret = biology_proxy_dev_create(config);
    if (ret) {
        goto err_file_close;
    }

    return 0;

err_file_close:
    bdev_fput(file);
err_devs_destroy:
    biology_proxy_devs_destroy();
err:
    return ret;
}

static void __exit biology_proxy_exit(void)
{
    bdev_fput(file);
    biology_proxy_devs_destroy();
}

module_init(biology_proxy_init);
module_exit(biology_proxy_exit);

module_param(target, charp, S_IRUGO);

MODULE_AUTHOR("Mikhail Peredriy");
MODULE_LICENSE("GPL");
