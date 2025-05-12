#ifndef BLGY_CLI_COMMON_H_
#define BLGY_CLI_COMMON_H_

#include <stdio.h>

#define BLGY_CLI_LOG(format, ...) printf(format "\n", ##__VA_ARGS__)

#define BLGY_CLI_ERR(format, ...) printf("error: " format "\n", ##__VA_ARGS__)

#endif
