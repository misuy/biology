#ifndef BLGY_GEN_WORKER_CTL_
#define BLGY_GEN_WORKER_CTL_

struct blgy_gen_group;

int blgy_gen_group_ctl_init(struct blgy_gen_group *group);
void blgy_gen_group_ctl_destroy(struct blgy_gen_group *group);

#endif
