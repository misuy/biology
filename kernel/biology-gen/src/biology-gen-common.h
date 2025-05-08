#ifndef BLGY_GEN_COMMON_H_
#define BLGY_GEN_COMMON_H_

#include <linux/printk.h>
#include <linux/err.h>
#include <linux/slab.h>

#define BLGY_GEN_MOD_NAME "biology-gen"

#define BLGY_GEN_PREFIX BLGY_GEN_MOD_NAME ": "

#define BLGY_GEN_INFO(fmt, args...) \
    pr_info(BLGY_GEN_PREFIX fmt "\n", ## args)

#define BLGY_GEN_WARN(fmt, args...) \
    pr_warn(BLGY_GEN_PREFIX fmt "\n", ## args)

#define BLGY_GEN_ERR(fmt, args...) \
    pr_err(BLGY_GEN_PREFIX fmt "\n", ## args)

#define BLGY_GEN_DBG(fmt, args...) \
    pr_debug(BLGY_GEN_PREFIX fmt "\n", ## args)

static inline ssize_t word_size(const char *buf, size_t cnt)
{
    size_t ptr = 0;
    while ((ptr < cnt) && buf[ptr] != ' ') {
        ptr++;
    }
    return ptr;
}

static inline int parse_word(char **buf, size_t *cnt, char **word)
{
    size_t size = word_size(*buf, *cnt);

    if (size < 0)
        return -EINVAL;

    *word = kzalloc(size + 1, GFP_KERNEL);
    if (!*word)
        return -EINVAL;

    memcpy(*word, *buf, size);

    *buf += size + 1;
    *cnt -= size + 1;

    return 0;
}

#endif
