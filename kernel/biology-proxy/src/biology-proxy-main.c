#include <linux/blkdev.h>
#include <linux/module.h>

#include "biology-proxy-common.h"
#include "biology-proxy-ctl.h"
#include "biology-proxy-dev.h"

static int __init biology_proxy_init(void)
{
    int ret;
    if ((ret = biology_proxy_devs_init())) {
        goto err;
    }

    if ((ret = biology_proxy_ctl_init())) {
        goto err_devs_destroy;
    }

    BIOLOGY_PROXY_INFO("module inserted");

    return 0;

err_devs_destroy:
    biology_proxy_devs_destroy();
err:
    return ret;
}

static void __exit biology_proxy_exit(void)
{
    biology_proxy_ctl_destroy();
    biology_proxy_devs_destroy();
    BIOLOGY_PROXY_INFO("module removed");
}

module_init(biology_proxy_init);
module_exit(biology_proxy_exit);

MODULE_AUTHOR("Mikhail Peredriy");
MODULE_LICENSE("GPL");
