#ifndef BLGY_LIB_PRXY_H_
#define BLGY_LIB_PRXY_H_

int blgy_lib_prxy_create_dev(char *name, char *device);
int blgy_lib_prxy_dev_enable(char *name, char *dump);
int blgy_lib_prxy_dev_disable(char *name);
int blgy_lib_prxy_dev_destroy(char *name);
int blgy_lib_prxy_dev_active(char *name);
int blgy_lib_prxy_dev_traced(char *name);
int blgy_lib_prxy_dev_inflight(char *name);

#endif
