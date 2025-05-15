#include "proxy.h"

#include <errno.h>

#include "include/biologylib/proxy.h"
#include "command.h"
#include "common.h"


BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_create, name);
BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_create, device);

static int proxy_create_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_create, name));
    if (!name)
        return -EINVAL;

    char *device = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_create, device));
    if (!device)
        return -EINVAL;

    int ret = blgy_lib_prxy_create_dev(name, device);
    if (ret < 0)
        BLGY_CLI_ERR("failed to create device (%s)", name);
    else
        BLGY_CLI_LOG("proxy (%s [device=%s]) created", name, device);
    return ret;
}

BLGY_COMMAND_DEFINE(
    proxy_create,
    proxy_create_handle,
    BLGY_ARG_FORMAT(proxy_create, name),
    BLGY_ARG_FORMAT(proxy_create, device),
    NULL
);


BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_enable, name);
BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_enable, dump);
BLGY_ARG_DEFINE_BOOL(proxy_enable, dump_payload, 0);

static int proxy_enable_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_enable, name));
    if (!name)
        return -EINVAL;

    char *dump = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_enable, dump));
    if (!dump)
        return -EINVAL;

    int *dump_payload_p = (int *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_enable, dump_payload));
    if (!dump_payload_p)
        return -EINVAL;

    int dump_payload = *dump_payload_p;

    int ret = blgy_lib_prxy_dev_enable(name, dump, dump_payload);
    if (ret < 0)
        BLGY_CLI_ERR("failed to enable proxy (%s)", name);
    else
        BLGY_CLI_LOG("proxy (%s) enabled", name);
    return ret;
}

BLGY_COMMAND_DEFINE(
    proxy_enable,
    proxy_enable_handle,
    BLGY_ARG_FORMAT(proxy_enable, name),
    BLGY_ARG_FORMAT(proxy_enable, dump),
    BLGY_ARG_FORMAT(proxy_enable, dump_payload),
    NULL
);


BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_disable, name);

static int proxy_disable_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_disable, name));
    if (!name)
        return -EINVAL;

    int ret = blgy_lib_prxy_dev_disable(name);
    if (ret < 0)
        BLGY_CLI_ERR("failed to disable proxy (%s)", name);
    else
        BLGY_CLI_LOG("proxy (%s) disabled", name);
    return ret;
}

BLGY_COMMAND_DEFINE(
    proxy_disable,
    proxy_disable_handle,
    BLGY_ARG_FORMAT(proxy_disable, name),
    NULL
);


BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_destroy, name);

static int proxy_destroy_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_destroy, name));
    if (!name)
        return -EINVAL;

    int ret = blgy_lib_prxy_dev_destroy(name);
    if (ret < 0)
        BLGY_CLI_ERR("failed to destroy proxy (%s)", name);
    else
        BLGY_CLI_LOG("proxy (%s) destroyed", name);
    return ret;
}

BLGY_COMMAND_DEFINE(
    proxy_destroy,
    proxy_destroy_handle,
    BLGY_ARG_FORMAT(proxy_destroy, name),
    NULL
);


BLGY_ARG_DEFINE_STRING_REQUIRED(proxy_info, name);

static int proxy_info_handle(struct blgy_cli_command *command)
{
    char *name = (char *)
        blgy_cli_command_arg_get(command->args,
                                 BLGY_ARG_FORMAT(proxy_info, name));
    if (!name)
        return -EINVAL;

    int active = blgy_lib_prxy_dev_active(name);
    if (active < 0) {
        BLGY_CLI_ERR("failed to get proxy (%s) active", name);
        return active;
    }

    int traced = blgy_lib_prxy_dev_traced(name);
    if (traced < 0) {
        BLGY_CLI_ERR("failed to get proxy (%s) traced", name);
        return traced;
    }

    int inflight = blgy_lib_prxy_dev_inflight(name);
    if (inflight < 0) {
        BLGY_CLI_ERR("failed to get proxy (%s) inflight", name);
        return inflight;
    }

    BLGY_CLI_LOG("proxy (%s[active=%d, traced=%d, inflight=%d])",
                 name, active, traced, inflight);
    return 0;
}

BLGY_COMMAND_DEFINE(
    proxy_info,
    proxy_info_handle,
    BLGY_ARG_FORMAT(proxy_info, name),
    NULL
);

BLGY_COMMAND_DEFINE(
    proxy_list,
    NULL,
    NULL
);
