#ifndef BLGY_PRXY_COMMON_H_
#define BLGY_PRXY_COMMON_H_

#include <linux/printk.h>

#define BLGY_PRXY_MOD_NAME "biology-proxy"
#define BLGY_PRXY_MOD_NAME_SHORT "bp"

#define BLGY_PRXY_PREFIX BLGY_PRXY_MOD_NAME ": "

#define BLGY_PRXY_INFO(fmt, args...) \
    pr_info(BLGY_PRXY_PREFIX fmt "\n", ## args)

#define BLGY_PRXY_WARN(fmt, args...) \
    pr_warn(BLGY_PRXY_PREFIX fmt "\n", ## args)

#define BLGY_PRXY_ERR(fmt, args...) \
    pr_err(BLGY_PRXY_PREFIX fmt "\n", ## args)

#define BLGY_PRXY_DBG(fmt, args...) \
    pr_debug(BLGY_PRXY_PREFIX fmt "\n", ## args)

static inline ssize_t first_word_size(const char *buf, size_t cnt)
{
    size_t ptr = 0;
    while ((ptr < cnt) && buf[ptr] != ' ') {
        ptr++;
    }

    if (ptr == cnt)
        return ptr;

    return ptr - 1;
}

#endif
