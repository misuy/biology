#include "biology-gen-parser.h"

#include <linux/memory.h>
#include <linux/blk_types.h>

#include "biology-gen-common.h"
#include "biology-gen-bio.h"
#include "biology-proxy-bio-serial.h"

static void
blgy_gen_parser_file_free(struct blgy_gen_parser_file *file)
{
    filp_close(file->file, 0);
    file->file = NULL;
}

static int
blgy_gen_parser_file_init(struct blgy_gen_parser_file *file, char *path)
{
    BLGY_GEN_DBG("opening dump file %s", path);
    file->offset = 0;
    file->file = filp_open(path, O_RDWR | O_CREAT | O_DSYNC, 0);
    if (!file->file) {
        BLGY_GEN_ERR("failed to open dump file %s", path);
        file->file = NULL;
        return -EINVAL;
    }
    return 0;
}

static size_t blgy_gen_parser_file_read(struct blgy_gen_parser_file *file,
                                        char *buf, size_t size)
{
    struct iov_iter iter;
    struct bio_vec bvec;

    BLGY_GEN_DBG("dump file read %lu", size);

    bvec.bv_page = virt_to_page(buf);
    bvec.bv_offset = offset_in_page(buf);
    bvec.bv_len = size;

    iov_iter_bvec(&iter, READ, &bvec, 1, size);

    return vfs_iter_read(file->file, &iter, &file->offset, 0);
}

int blgy_gen_parser_init(struct blgy_gen_parser *parser, char *path)
{
    int ret;

    if ((ret = blgy_gen_parser_file_init(&parser->file, path)))
        return ret;

    parser->size = BLGY_GEN_PARSER_INITIAL_BUF_SIZE;
    parser->buf = kmalloc(parser->size, GFP_KERNEL);
    parser->offset = 0;
    parser->valid =
        blgy_gen_parser_file_read(&parser->file, parser->buf, parser->size);

    if (parser->valid == 0) {
        BLGY_GEN_ERR("got invalid dump file");
        blgy_gen_parser_file_free(&parser->file);
        return -EINVAL;
    }

    parser->offset +=
        blgy_prxy_bio_serial_schema_deserialize(&parser->schema, parser->buf);

    return 0;
}

static void
blgy_gen_parser_refill_buf(struct blgy_gen_parser *parser, size_t requested)
{
    size_t read;
    size_t save = blgy_gen_parser_buf_avail(parser);

    memcpy(parser->buf, blgy_gen_parser_buf_pos(parser),
           save);
    parser->offset = 0;

    if (parser->size < requested) {
        parser->buf = krealloc(parser->buf, requested, GFP_KERNEL);
        parser->size = requested;
    }

    read = blgy_gen_parser_file_read(&parser->file, parser->buf + save,
                                     parser->size - save);

    parser->valid = read + save;
}

enum blgy_gen_parser_read_next_ret
blgy_gen_parser_read_next(struct blgy_gen_parser *parser,
                          struct blgy_prxy_bio_info *info)
{
    size_t size;

    if (blgy_gen_parser_buf_avail(parser) < sizeof(size_t))
        blgy_gen_parser_refill_buf(parser, sizeof(size_t));

    if (blgy_gen_parser_buf_avail(parser) < sizeof(size_t))
        return BLGY_GEN_PARSER_READ_NEXT_EMPTY;

    size = blgy_prxy_bio_serialized_size(blgy_gen_parser_buf_pos(parser));
    if (blgy_gen_parser_buf_avail(parser) < size) {
        blgy_gen_parser_refill_buf(parser, size);
    }

    if (blgy_gen_parser_buf_avail(parser) < size) {
        return BLGY_GEN_PARSER_READ_NEXT_ERR;
    }

    parser->offset += blgy_prxy_bio_deserialize(info,
                                                blgy_gen_parser_buf_pos(parser),
                                                parser->schema);

    return BLGY_GEN_PARSER_READ_NEXT_OK;
}

void blgy_gen_parser_free(struct blgy_gen_parser *parser)
{
    kfree(parser->buf);
    parser->buf = NULL;
    blgy_gen_parser_file_free(&parser->file);
}
