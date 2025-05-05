#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "info.h"

char *build_file_path(char *dir_path, char *file_name)
{
    char *file_path = malloc(strlen(dir_path) + strlen(file_name) + 2);
    if (!file_path)
        return NULL;

    char *file_path_ptr = file_path;
    memcpy(file_path_ptr, dir_path, strlen(dir_path));
    file_path_ptr += strlen(dir_path);

    *file_path_ptr = '/';
    file_path_ptr++;

    memcpy(file_path_ptr, file_name, strlen(file_name));
    file_path_ptr += strlen(file_name);

    *file_path_ptr = 0;

    return file_path;
}

int main(int argc, char **argv)
{
    printf("hiii\n");
    int ret = 0;
    char *dir_path = argv[1];
    DIR *dir = opendir(dir_path);
    if (!dir) {
        printf("invalid dir\n");
        return -EINVAL;
    }

    struct dirent *ent;
    while ((ent = readdir(dir))) {
        printf("%s\n", ent->d_name);
        if (ent->d_type == DT_REG) {
            char *file_path = build_file_path(dir_path, ent->d_name);
            printf("%s", file_path);
            if (!file_path) {
                ret = -ENOMEM;
                goto end;
            }

            if ((ret = parse_dump_file(file_path)) < 0)
                goto end;
        }
    }

end:
    closedir(dir);
    return ret;
}
