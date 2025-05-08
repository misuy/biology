#ifndef BIOLOGY_GEN_PARSER_H_
#define BIOLOGY_GEN_PARSER_H_

#include <linux/types.h>

struct blgy_prxy_bio_info;

#define BLGY_GEN_PARSER_INITIAL_BUF_SIZE 4096

struct blgy_gen_parser_file {
    struct file *file;
    loff_t offset;
};

struct blgy_gen_parser {
    struct blgy_gen_parser_file file;
    struct blgy_prxy_bio_serial_schema_field **schema;
    char *buf;
    size_t size;
    size_t valid;
    size_t offset;
};

#define blgy_gen_parser_buf_avail(parser) \
    (parser->valid - parser->offset)

#define blgy_gen_parser_buf_pos(parser) \
    (parser->buf + parser->offset)

enum blgy_gen_parser_read_next_ret {
    BLGY_GEN_PARSER_READ_NEXT_OK = 0,
    BLGY_GEN_PARSER_READ_NEXT_EMPTY,
    BLGY_GEN_PARSER_READ_NEXT_ERR,
};

int blgy_gen_parser_init(struct blgy_gen_parser *parser, char *path);

void blgy_gen_parser_free(struct blgy_gen_parser *parser);

enum blgy_gen_parser_read_next_ret
blgy_gen_parser_read_next(struct blgy_gen_parser *parser,
                          struct blgy_prxy_bio_info *info);

#endif
