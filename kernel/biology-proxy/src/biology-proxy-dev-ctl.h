#ifndef BLGY_PRXY_DEV_CTL_H_
#define BLGY_PRXY_DEV_CTL_H_

struct blgy_prxy_dev;

int blgy_prxy_dev_ctl_init(struct blgy_prxy_dev *dev);
void blgy_prxy_dev_ctl_destroy(struct blgy_prxy_dev *dev);

#endif