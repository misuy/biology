#ifndef JSON_H_
#define JSON_H_

struct blgy_prxy_bio_info;

void json_bio_info_begin(void);
void json_bio_info_end(void);
void json_bio_info(struct blgy_prxy_bio_info *info, int first);

#endif
