#ifndef BLGY_CLI_COMMAND_H_
#define BLGY_CLI_COMMAND_H_

#include <stddef.h>

enum blgy_cli_command_arg_type {
    BLGY_CLI_COMMAND_ARG_TYPE_BOOL,
    BLGY_CLI_COMMAND_ARG_TYPE_INT,
    BLGY_CLI_COMMAND_ARG_TYPE_STRING,
};

struct blgy_cli_command_arg_def {
    enum blgy_cli_command_arg_type type;
    char *name;
};

struct blgy_cli_command_arg {
    struct blgy_cli_command_arg_def *def;
    void *value;
};

struct blgy_cli_command;

typedef int (*blgy_cli_command_handle_fn) (struct blgy_cli_command *command);

struct blgy_cli_command_def {
    char *name;
    struct blgy_cli_command_arg_def **args;
    blgy_cli_command_handle_fn handle;
};

struct blgy_cli_command {
    struct blgy_cli_command_def *def;
    struct blgy_cli_command_arg **args;
};

#define BLGY_ARG_NAME(_command, _name) _command##_arg_def_##_name

#define BLGY_ARG_DEFINE(_command, _type, _name)                         \
    struct blgy_cli_command_arg_def BLGY_ARG_NAME(_command, _name) = {  \
        .type = _type,                                                  \
        .name = #_name,                                                 \
    };                                                                  \

#define BLGY_ARG_FORMAT(_command, _name) &BLGY_ARG_NAME(_command, _name)

#define BLGY_ARG_LIST_NAME(_command) _command##_arg_list

#define BLGY_ARG_LIST_FORMAT(...) { __VA_ARGS__ }

#define BLGY_ARG_LIST_DEFINE(_command, ...) \
    struct blgy_cli_command_arg_def *BLGY_ARG_LIST_NAME(_command)[] = BLGY_ARG_LIST_FORMAT(__VA_ARGS__);

#define BLGY_COMMAND_NAME(_command) _command##_command_def

#define BLGY_COMMAND_DEFINE(_command, _handle, ...)             \
    BLGY_ARG_LIST_DEFINE(_command, __VA_ARGS__)                 \
                                                                \
    struct blgy_cli_command_def BLGY_COMMAND_NAME(_command) = { \
        .name = #_command,                                      \
        .handle = _handle,                                      \
        .args = BLGY_ARG_LIST_NAME(_command),                   \
    };                                                          \

#define BLGY_COMMAND_FORMAT(_command) &BLGY_COMMAND_NAME(_command)

#define BLGY_COMMAND_EXTERN(_command) extern struct blgy_cli_command_def BLGY_COMMAND_NAME(_command)

void * blgy_cli_command_arg_get(struct blgy_cli_command_arg **args,
                                struct blgy_cli_command_arg_def *def);

int blgy_cli_command_handle(int argc, char **argv);

#endif
