#ifndef BLGY_PRXY_BIO_H_
#define BLGY_PRXY_BIO_H_

#include <linux/types.h>

#include "biology-proxy-dev.h"

struct blgy_prxy_bio_info {
    uint32_t id;
    ktime_t start_ts;
    ktime_t end_ts;
    sector_t sector;
    int cpu;
    unsigned int size;
};

struct blgy_prxy_bio {
    struct bio *bio;
    struct blgy_prxy_bio_info info;
    struct blgy_prxy_dev *dev;
    struct work_struct dump_work;
};

struct blgy_prxy_bio_serial_schema_field;

typedef size_t (*serial_schema_field_size_fn) (struct blgy_prxy_bio *bio);
typedef void * (*serial_schema_field_ptr_fn) (struct blgy_prxy_bio *bio);
typedef void (*serial_schema_field_serialize_fn) (
    struct blgy_prxy_bio_serial_schema_field *schema,
    struct blgy_prxy_bio *bio,
    void *buf
);

struct blgy_prxy_bio_serial_schema_field {
    serial_schema_field_size_fn size;
    serial_schema_field_ptr_fn ptr;
    serial_schema_field_serialize_fn serialize;
};

#define blgy_prxy_bio_serial_schema_field_size_fn_default(_field) \
    return sizeof(bio->_field);

#define blgy_prxy_bio_serial_schema_field_ptr_fn_default(_field) \
    return &(bio->_field);

#define define_blgy_prxy_bio_serial_schema_field(_name, _field,                 \
                                                 _size_fn, _ptr_fn)             \
    static size_t                                                               \
    field_##_name##_size(struct blgy_prxy_bio *bio)                             \
    {                                                                           \
        _size_fn(_field);                                                       \
    }                                                                           \
                                                                                \
    static void *                                                               \
    field_##_name##_ptr(struct blgy_prxy_bio *bio)                              \
    {                                                                           \
        _ptr_fn(_field)                                                         \
    }                                                                           \
                                                                                \
    static void                                                                 \
    field_##_name##_serialize(struct blgy_prxy_bio_serial_schema_field *schema, \
                              struct blgy_prxy_bio *bio, void *buf)             \
    {                                                                           \
        memcpy(buf, schema->ptr(bio), schema->size(bio));                       \
    }                                                                           \
                                                                                \
    struct blgy_prxy_bio_serial_schema_field field_##_name = {                  \
        .size = field_##_name##_size,                                           \
        .ptr = field_##_name##_ptr,                                             \
        .serialize = field_##_name##_serialize,                                 \
    };                                                                          \

#define declare_blgy_prxy_bio_serial_schema_field(_name)                        \
    &field_##_name                                                              \

void blgy_prxy_process_bio(struct blgy_prxy_dev *dev,
                                   struct bio *bio);

ssize_t blgy_prxy_bio_serialize(struct blgy_prxy_bio *bio, char **buf_p);

#endif
