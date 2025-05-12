#include "gen.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define BLGY_LIB_GEN_SYSFS_ROOT "/sys/module/biology_gen"
#define BLGY_LIB_GEN_SYSFS_MODULE BLGY_LIB_GEN_SYSFS_ROOT "/module"
#define BLGY_LIB_GEN_SYSFS_MODULE_CTL BLGY_LIB_GEN_SYSFS_MODULE "/ctl"
#define BLGY_LIB_GEN_SYSFS_WORKER BLGY_LIB_GEN_SYSFS_ROOT "/%s"
#define BLGY_LIB_GEN_SYSFS_WORKER_CTL BLGY_LIB_GEN_SYSFS_WORKER "/ctl"

static FILE * blgy_lib_gen_open_ctl(void)
{
    return fopen(BLGY_LIB_GEN_SYSFS_MODULE_CTL, "w");
}

static FILE * blgy_lib_gen_open_worker_ctl(char *name)
{
    char *ctl_path = malloc(PATH_MAX);
    if (!ctl_path)
        return NULL;

    sprintf(ctl_path, BLGY_LIB_GEN_SYSFS_WORKER_CTL, name);
    FILE *ctl = fopen(ctl_path, "w");
    free(ctl_path);
    return ctl;
}

#define BLGY_LIB_GEN_START_WORKER_CMD "start %s %s %s %d"

int blgy_lib_gen_start_worker(char *name, char *device, char *dump, int cpucnt)
{
    if (!blgy_lib_dir_exists(dump))
        return -EINVAL;

    FILE *ctl = blgy_lib_gen_open_ctl();
    if (!ctl)
        return -ENOENT;

    int ret = fprintf(ctl, BLGY_LIB_GEN_START_WORKER_CMD, name, device, dump,
                      cpucnt);
    fclose(ctl);
    return ret;
}

#define BLGY_LIB_GEN_WORKER_STOP_CMD "stop"

int blgy_lib_gen_worker_stop(char *name)
{
    FILE *ctl = blgy_lib_gen_open_worker_ctl(name);
    if (!ctl)
        return -ENOENT;

    int ret = fprintf(ctl, BLGY_LIB_GEN_WORKER_STOP_CMD);
    fclose(ctl);
    return ret;
}

int blgy_lib_gen_worker_alive(char *name)
{
    FILE *ctl = blgy_lib_gen_open_worker_ctl(name);
    int ret = ctl != NULL;
    if (ret)
        fclose(ctl);
    return ret;
}
