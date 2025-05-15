#include <stdlib.h>

#include "command.h"
#include "common.h"

int main(int argc, char **argv)
{
    argc--;
    argv++;

    int ret = blgy_cli_command_handle(argc, argv);
    if (ret < 0) {
        BLGY_CLI_ERR("command failed");
        exit(ret);
    }

    exit(0);
}
