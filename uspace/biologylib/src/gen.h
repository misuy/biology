#ifndef BLGY_LIB_GEN_H_
#define BLGY_LIB_GEN_H_

int blgy_lib_gen_start_worker(char *name, char *device, char *dump, int cpucnt);
int blgy_lib_gen_worker_stop(char *name);
int blgy_lib_gen_worker_alive(char *name);

#endif
