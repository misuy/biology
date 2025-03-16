#ifndef BIOLOGY_PROXY_COMMON_H_
#define BIOLOGY_PROXY_COMMON_H_

#include <linux/printk.h>

#define BIOLOGY_PROXY_MOD_NAME "biology-proxy"
#define BIOLOGY_PROXY_MOD_NAME_SHORT "bp"

#define BIOLOGY_PROXY_PREFIX BIOLOGY_PROXY_MOD_NAME ": "

#define BIOLOGY_PROXY_INFO(fmt, args...) \
    pr_info(BIOLOGY_PROXY_PREFIX fmt "\n", ## args)

#define BIOLOGY_PROXY_WARN(fmt, args...) \
    pr_warn(BIOLOGY_PROXY_PREFIX fmt "\n", ## args)

#define BIOLOGY_PROXY_ERR(fmt, args...) \
    pr_err(BIOLOGY_PROXY_PREFIX fmt "\n", ## args)

#define BIOLOGY_PROXY_DBG(fmt, args...) \
    pr_debug(BIOLOGY_PROXY_PREFIX fmt "\n", ## args)


#define BIOLOGY_PROXY_DEV_DIR BIOLOGY_PROXY_MOD_NAME_SHORT

#endif
