#include "command.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "common.h"
#include "proxy.h"
#include "gen.h"

void * blgy_cli_command_arg_get(struct blgy_cli_command_arg **args,
                                struct blgy_cli_command_arg_def *def)
{
    struct blgy_cli_command_arg **arg = args;
    while (*arg != NULL) {
        if ((*arg)->def == def) {
            return (*arg)->value;
        }
        arg++;
    }
    return NULL;
}

static int blgy_cli_command_def_args_cnt(struct blgy_cli_command_def *def)
{
    struct blgy_cli_command_arg_def **arg = def->args;
    while (*arg != NULL) {
        arg++;
    }
    return arg - def->args;
}

static int blgy_cli_command_def_compare_name(struct blgy_cli_command_def *def,
                                             int argc, char **argv)
{
    int cnt = 1, i = 0;
    while (def->name[i] != 0) {
        if (def->name[i] == '_')
            cnt++;
        i++;
    }

    if (argc < cnt)
        return 0;

    for (i = 0; i < cnt; i++) {
        if (strstr(def->name, argv[i]) == NULL)
            return 0;
    }

    return cnt;
}

static struct blgy_cli_command_def *
blgy_cli_command_def_parse(struct blgy_cli_command_def **defs,
                           int *argc, char ***argv)
{
    int ret = 0, i = 0;
    struct blgy_cli_command_def **def = defs;
    while (*def != NULL) {
        if ((ret = blgy_cli_command_def_compare_name(*def, *argc, *argv)) != 0) {
            *argc -= ret;
            *argv += ret;
            return *def;
        }
        def++;
    }

    BLGY_CLI_ERR("invalid command");
    return NULL;
}

static int blgy_cli_arg_parse(struct blgy_cli_command_arg *arg,
                              int argc, char **argv)
{
    printf("parsing %s, %d\n", arg->def->name, argc);
    int i = 0;
    while (i < (argc - 1)) {
        if (strcmp(argv[i], arg->def->name) == 0) {
            i++;
            switch (arg->def->type) {
                case BLGY_CLI_COMMAND_ARG_TYPE_BOOL:
                    arg->value = malloc(sizeof(int));
                    if ((strcmp(argv[i], "0") == 0)
                        || (strcmp(argv[i], "false") == 0)) {
                        *((int *) arg->value) = 0;
                    }
                    else {
                        *((int *) arg->value) = 1;
                    }
                    break;

                case BLGY_CLI_COMMAND_ARG_TYPE_INT:
                    arg->value = malloc(sizeof(int));
                    *((int *) arg->value) = atoi(argv[i]);
                    break;

                case BLGY_CLI_COMMAND_ARG_TYPE_STRING:
                    arg->value = malloc(strlen(argv[i]));
                    strcpy(arg->value, argv[i]);
                    break;
            }

            return 0;
        }

        i++;
    }

    if (!arg->def->required) {
        char *s = NULL;
        switch (arg->def->type) {
            case BLGY_CLI_COMMAND_ARG_TYPE_BOOL:
            case BLGY_CLI_COMMAND_ARG_TYPE_INT:
                arg->value = malloc(sizeof(int));
                memcpy(arg->value, arg->def->default_value, sizeof(int));
                break;

            case BLGY_CLI_COMMAND_ARG_TYPE_STRING:
                s = *((char **) arg->def->default_value);
                arg->value = malloc(strlen(s));
                strcpy(arg->value, s);
                break;
        }

        return 0;
    }

    return -EINVAL;
}

static struct blgy_cli_command *
blgy_cli_command_parse(struct blgy_cli_command_def *def, int argc, char **argv)
{
    int ret = 0;
    struct blgy_cli_command *command = malloc(sizeof(struct blgy_cli_command));
    command->def = def;
    command->args =
        malloc(sizeof(struct blgy_cli_command_arg *)
               * (blgy_cli_command_def_args_cnt(def) + 1));

    int i = 0;
    while (def->args[i] != NULL) {
        command->args[i] = malloc(sizeof(struct blgy_cli_command_arg));
        command->args[i]->def = def->args[i];
        command->args[i]->value = NULL;
        if ((ret = blgy_cli_arg_parse(command->args[i], argc, argv))) {
            BLGY_CLI_ERR("failed to parse arg (%s) for command (%s)",
                         command->args[i]->def->name, command->def->name);
            return NULL;
        }
        printf("argument %s parsed\n", command->def->name);
        i++;
    }

    command->args[i] = NULL;
    return command;
}

static void blgy_cli_command_free(struct blgy_cli_command *command)
{
    struct blgy_cli_command_arg **arg = command->args;
    while (*arg != NULL) {
        free((*arg)->value);
        free(*arg);
        arg++;
    }
    free(command->args);
    free(command);
}

struct blgy_cli_command_def *commands[] = {
    BLGY_COMMAND_FORMAT(proxy_create),
    BLGY_COMMAND_FORMAT(proxy_enable),
    BLGY_COMMAND_FORMAT(proxy_disable),
    BLGY_COMMAND_FORMAT(proxy_destroy),
    BLGY_COMMAND_FORMAT(proxy_info),
    BLGY_COMMAND_FORMAT(gen_start),
    BLGY_COMMAND_FORMAT(gen_stop),
    NULL
};

int blgy_cli_command_handle(int argc, char **argv)
{
    struct blgy_cli_command_def *def =
        blgy_cli_command_def_parse(commands, &argc, &argv);
    if (!def)
        return -EINVAL;

    struct blgy_cli_command *command = blgy_cli_command_parse(def, argc, argv);
    if (!command)
        return -EINVAL;

    int ret = command->def->handle(command);
    blgy_cli_command_free(command);
    return ret;
}
