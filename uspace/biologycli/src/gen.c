#include "command.h"

#include <errno.h>
#include <unistd.h>

#include "include/biologylib/gen.h"
#include "common.h"

BLGY_ARG_DEFINE_STRING_REQUIRED(gen_start, name);
BLGY_ARG_DEFINE_STRING_REQUIRED(gen_start, device);
BLGY_ARG_DEFINE_STRING_REQUIRED(gen_start, dump);
BLGY_ARG_DEFINE_INT_REQUIRED(gen_start, cpus);
BLGY_ARG_DEFINE_BOOL(gen_start, wait, 1);

static int gen_create_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(gen_start, name));
    if (!name)
        return -EINVAL;

    char *device = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(gen_start, device));
    if (!device)
        return -EINVAL;

    char *dump = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(gen_start, dump));
    if (!device)
        return -EINVAL;

    int *cpusp = (int *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(gen_start, cpus));
    if (!cpusp)
        return -EINVAL;
    int cpus = *cpusp;

    int *waitp = (int *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(gen_start, wait));
    if (!waitp)
        return -EINVAL;
    int wait = *waitp;


    int ret = blgy_lib_gen_start_worker(name, device, dump, cpus);
    if (ret < 0) {
        BLGY_CLI_ERR("failed to create gen worker (%s)", name);
        goto end;
    }
    else
        BLGY_CLI_LOG("gen worker (%s [device=%s]) created", name, device);

    if (wait) {
        BLGY_CLI_LOG("waiting for worker (%s) to complete", name);
        while (blgy_lib_gen_worker_alive(name))
            usleep(300);

        BLGY_CLI_LOG("worker (%s) completed", name);
    }

end:
    return ret;
}

BLGY_COMMAND_DEFINE(
    gen_start,
    gen_create_handle,
    BLGY_ARG_FORMAT(gen_start, name),
    BLGY_ARG_FORMAT(gen_start, device),
    BLGY_ARG_FORMAT(gen_start, dump),
    BLGY_ARG_FORMAT(gen_start, cpus),
    BLGY_ARG_FORMAT(gen_start, wait),
    NULL
);


BLGY_ARG_DEFINE_STRING_REQUIRED(gen_stop, name);

static int gen_stop_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(gen_stop, name));
    if (!name)
        return -EINVAL;

    int ret = blgy_lib_gen_worker_stop(name);
    if (ret < 0)
        BLGY_CLI_ERR("failed to stop gen worker (%s)", name);
    else
        BLGY_CLI_LOG("gen worker (%s) stopped", name);
    return ret;
}

BLGY_COMMAND_DEFINE(
    gen_stop,
    gen_stop_handle,
    BLGY_ARG_FORMAT(gen_stop, name),
    NULL
);
