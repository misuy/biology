#include "proxy.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define BLGY_LIB_PRXY_SYSFS_ROOT "/sys/module/biology_proxy"
#define BLGY_LIB_PRXY_SYSFS_MODULE BLGY_LIB_PRXY_SYSFS_ROOT "/module"
#define BLGY_LIB_PRXY_SYSFS_MODULE_CTL BLGY_LIB_PRXY_SYSFS_MODULE "/ctl"
#define BLGY_LIB_PRXY_SYSFS_DEV BLGY_LIB_PRXY_SYSFS_ROOT "/%s"
#define BLGY_LIB_PRXY_SYSFS_DEV_CTL BLGY_LIB_PRXY_SYSFS_DEV "/ctl"
#define BLGY_LIB_PRXY_SYSFS_DEV_ACTIVE BLGY_LIB_PRXY_SYSFS_DEV "/active"
#define BLGY_LIB_PRXY_SYSFS_DEV_TRACED BLGY_LIB_PRXY_SYSFS_DEV "/traced"
#define BLGY_LIB_PRXY_SYSFS_DEV_INFLIGHT BLGY_LIB_PRXY_SYSFS_DEV "/inflight"

static FILE * blgy_lib_prxy_open_ctl(void)
{
    return fopen(BLGY_LIB_PRXY_SYSFS_MODULE_CTL, "w");
}

static FILE * blgy_lib_prxy_open_dev_ctl(char *name)
{
    char *ctl_path = malloc(PATH_MAX);
    if (!ctl_path)
        return NULL;

    sprintf(ctl_path, BLGY_LIB_PRXY_SYSFS_DEV_CTL, name);
    FILE *ctl = fopen(ctl_path, "w");
    free(ctl_path);
    return ctl;
}

static FILE * blgy_lib_prxy_open_dev_active(char *name)
{
    char *active_path = malloc(PATH_MAX);
    if (!active_path)
        return NULL;

    sprintf(active_path, BLGY_LIB_PRXY_SYSFS_DEV_ACTIVE, name);
    FILE *active = fopen(active_path, "r");
    free(active_path);
    return active;
}

static FILE * blgy_lib_prxy_open_dev_traced(char *name)
{
    char *traced_path = malloc(PATH_MAX);
    if (!traced_path)
        return NULL;

    sprintf(traced_path, BLGY_LIB_PRXY_SYSFS_DEV_TRACED, name);
    FILE *traced = fopen(traced_path, "r");
    free(traced_path);
    return traced;
}

static FILE * blgy_lib_prxy_open_dev_inflight(char *name)
{
    char *inflight_path = malloc(PATH_MAX);
    if (!inflight_path)
        return NULL;

    sprintf(inflight_path, BLGY_LIB_PRXY_SYSFS_DEV_INFLIGHT, name);
    FILE *inflight = fopen(inflight_path, "r");
    free(inflight_path);
    return inflight;
}

#define BLGY_LIB_PRXY_CREATE_DEV_CMD "create %s %s"

int blgy_lib_prxy_create_dev(char *name, char *device)
{
    FILE *ctl = blgy_lib_prxy_open_ctl();
    if (ctl == NULL)
        return -ENOENT;

    int ret = fprintf(ctl, BLGY_LIB_PRXY_CREATE_DEV_CMD, name, device);
    fclose(ctl);
    return ret;
}

#define BLGY_LIB_PRXY_DEV_ENABLE_CMD "enable %s"

int blgy_lib_prxy_dev_enable(char *name, char *dump)
{
    int ret = 0;
    FILE *ctl = blgy_lib_prxy_open_dev_ctl(name);
    if (!ctl)
        return -ENOENT;

    if (!blgy_lib_dir_exists(dump)) {
        if ((ret = blgy_lib_create_dir(dump))) {
            fclose(ctl);
            return ret;
        }
    }

    ret = fprintf(ctl, BLGY_LIB_PRXY_DEV_ENABLE_CMD, dump);
    fclose(ctl);
    return 0;
}

#define BLGY_LIB_PRXY_DEV_DISABLE_CMD "disable"

int blgy_lib_prxy_dev_disable(char *name)
{
    FILE *ctl = blgy_lib_prxy_open_dev_ctl(name);
    if (!ctl)
        return -ENOENT;

    int ret = fprintf(ctl, BLGY_LIB_PRXY_DEV_DISABLE_CMD);
    fclose(ctl);
    return ret;
}

#define BLGY_LIB_PRXY_DEV_DESTROY_CMD "destroy"

int blgy_lib_prxy_dev_destroy(char *name)
{
    FILE *ctl = blgy_lib_prxy_open_dev_ctl(name);
    if (!ctl)
        return -ENOENT;

    int ret = fprintf(ctl, BLGY_LIB_PRXY_DEV_DESTROY_CMD);
    fclose(ctl);
    return ret;
}

int blgy_lib_prxy_dev_active(char *name)
{
    FILE *ctl = blgy_lib_prxy_open_dev_active(name);
    if (!ctl)
        return -ENOENT;

    int size = getpagesize();
    char *buf = malloc(size);
    memset(buf, 0, size);

    fread(buf, size, 1, ctl);
    int ret = atoi(buf);

    free(buf);
    fclose(ctl);

    return ret;
}

int blgy_lib_prxy_dev_traced(char *name)
{
    FILE *ctl = blgy_lib_prxy_open_dev_traced(name);
    if (!ctl)
        return -ENOENT;

    int size = getpagesize();
    char *buf = malloc(size);
    memset(buf, 0, size);

    fread(buf, size, 1, ctl);
    int ret = atoi(buf);

    free(buf);
    fclose(ctl);

    return ret;
}

int blgy_lib_prxy_dev_inflight(char *name)
{
    FILE *ctl = blgy_lib_prxy_open_dev_inflight(name);
    if (!ctl)
        return -ENOENT;

    int size = getpagesize();
    char *buf = malloc(size);
    memset(buf, 0, size);

    fread(buf, size, 1, ctl);
    int ret = atoi(buf);

    free(buf);
    fclose(ctl);

    return ret;
}
