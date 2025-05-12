#include "common.h"
#include <dirent.h>
#define _XOPEN_SOURCE 500
#include <ftw.h>

int blgy_lib_dir_exists(char *path)
{
    struct stat st;
    return stat(path, &st) != -1;
}

int blgy_lib_create_dir(char *path)
{
    return mkdir(path, S_IRWXU);
}
