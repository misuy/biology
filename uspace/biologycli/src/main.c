#include "command.h"

int main(int argc, char **argv)
{
    argc++;
    argv++;

    return blgy_cli_command_handle(argc, argv);
}
