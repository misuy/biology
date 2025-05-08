#include <linux/module.h>

#include "biology-gen-common.h"
#include "biology-gen-ctl.h"
#include "biology-gen-worker.h"

static int __init blgy_gen_init(void)
{
    int ret;
    if ((ret = blgy_gen_groups_init())) {
        goto err;
    }

    if ((ret = blgy_gen_ctl_init())) {
        goto err_groups_destroy;
    }

    BLGY_GEN_INFO("module inited");

    return 0;

err_groups_destroy:
    blgy_gen_groups_destroy();
err:
    return ret;
}

static void __exit blgy_gen_exit(void)
{
    blgy_gen_ctl_destroy();
    blgy_gen_groups_destroy();
    BLGY_GEN_INFO("module removed");
}

module_init(blgy_gen_init);
module_exit(blgy_gen_exit);

MODULE_AUTHOR("Mikhail Peredriy");
MODULE_LICENSE("GPL");
