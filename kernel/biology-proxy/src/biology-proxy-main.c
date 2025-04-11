#include <linux/blkdev.h>
#include <linux/module.h>

#include "biology-proxy-common.h"
#include "biology-proxy-ctl.h"
#include "biology-proxy-dev.h"

static int __init blgy_prxy_init(void)
{
    int ret;
    if ((ret = blgy_prxy_devs_init())) {
        goto err;
    }

    if ((ret = blgy_prxy_ctl_init())) {
        goto err_devs_destroy;
    }

    BLGY_PRXY_INFO("module inited");

    return 0;

err_devs_destroy:
    blgy_prxy_devs_destroy();
err:
    return ret;
}

static void __exit blgy_prxy_exit(void)
{
    blgy_prxy_ctl_destroy();
    blgy_prxy_devs_destroy();
    BLGY_PRXY_INFO("module removed");
}

module_init(blgy_prxy_init);
module_exit(blgy_prxy_exit);

MODULE_AUTHOR("Mikhail Peredriy");
MODULE_LICENSE("GPL");
