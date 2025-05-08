#ifndef BLGY_PRXY_BIO_SERIAL_H_
#define BLGY_PRXY_BIO_SERIAL_H_

#define C_PRXY
// #define C_GEN
// #define C_TO_JSON

#ifdef C_PRXY
#include <linux/types.h>
#include <linux/memory.h>

#include "biology-proxy-common.h"
#include "biology-proxy-bio.h"

#define LOG_ERR_OVERRIDE(fmt, args...) \
    BLGY_PRXY_ERR(fmt, ## args)

#define MALLOC_OVERRIDE(size) \
    kmalloc(size, GFP_KERNEL)

#define FREE_OVERRIDE(ptr) \
    kfree(ptr)
#endif

#ifdef C_GEN
#include <linux/types.h>
#include <linux/memory.h>

#include "biology-gen-common.h"
#include "biology-gen-bio.h"

#define LOG_ERR_OVERRIDE(fmt, args...) \
    BLGY_GEN_ERR(fmt, ## args)

#define MALLOC_OVERRIDE(size) \
    kmalloc(size, GFP_KERNEL)

#define FREE_OVERRIDE(ptr) \
    kfree(ptr)
#endif

#ifdef C_TO_JSON
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>

#include "bio-info.h"

#define LOG_ERR_OVERRIDE(fmt, args...) \
    printf("err: " fmt "\n", ## args)

#define MALLOC_OVERRIDE(size) \
    malloc(size)

#define FREE_OVERRIDE(ptr) \
    free(ptr)
#endif

struct blgy_prxy_bio_info;
struct blgy_prxy_bio_serial_schema_field;

typedef size_t (*serial_schema_field_size_fn) (struct blgy_prxy_bio_info *bio);
typedef void * (*serial_schema_field_ptr_fn) (struct blgy_prxy_bio_info *bio);
typedef void (*serial_schema_field_serialize_fn) (
    struct blgy_prxy_bio_serial_schema_field *schema,
    struct blgy_prxy_bio_info *bio,
    void *buf
);
typedef void (*serial_schema_field_deserialize_fn) (
    struct blgy_prxy_bio_serial_schema_field *schema,
    struct blgy_prxy_bio_info *bio,
    void *buf
);

struct blgy_prxy_bio_serial_schema_field {
    uint32_t id;
    serial_schema_field_size_fn size;
    serial_schema_field_ptr_fn ptr;
    serial_schema_field_serialize_fn serialize;
    serial_schema_field_deserialize_fn deserialize;
};

#define blgy_prxy_bio_serial_schema_field_size_fn_default(_field) \
    return sizeof(info->_field);

#define blgy_prxy_bio_serial_schema_field_ptr_fn_default(_field) \
    return &(info->_field);

#define blgy_prxy_bio_serial_schema_field_serialize_fn_default(_field) \
    memcpy(buf, schema->ptr(info), schema->size(info));

#define blgy_prxy_bio_serial_schema_field_deserialize_fn_default(_field) \
    memcpy(schema->ptr(info), buf, schema->size(info));

#define define_blgy_prxy_bio_serial_schema_field(_id, _name, _field,            \
                                                 _size_fn, _ptr_fn,             \
                                                 _serialize_fn,                 \
                                                 _deserialize_fn)               \
    static size_t                                                               \
    field_##_name##_size(struct blgy_prxy_bio_info *info)                       \
    {                                                                           \
        _size_fn(_field);                                                       \
    }                                                                           \
                                                                                \
    static void *                                                               \
    field_##_name##_ptr(struct blgy_prxy_bio_info *info)                        \
    {                                                                           \
        _ptr_fn(_field)                                                         \
    }                                                                           \
                                                                                \
    static void                                                                 \
    field_##_name##_serialize(struct blgy_prxy_bio_serial_schema_field *schema, \
                              struct blgy_prxy_bio_info *info, void *buf)       \
    {                                                                           \
        _serialize_fn(_field)                                                   \
    }                                                                           \
                                                                                \
    static void                                                                 \
    field_##_name##_deserialize(                                                \
        struct blgy_prxy_bio_serial_schema_field *schema,                       \
        struct blgy_prxy_bio_info *info, void *buf                              \
    )                                                                           \
    {                                                                           \
        _deserialize_fn(_field)                                                 \
    }                                                                           \
                                                                                \
    struct blgy_prxy_bio_serial_schema_field field_##_name = {                  \
        .id = _id,                                                              \
        .size = field_##_name##_size,                                           \
        .ptr = field_##_name##_ptr,                                             \
        .serialize = field_##_name##_serialize,                                 \
        .deserialize = field_##_name##_deserialize,                             \
    };                                                                          \

#define declare_blgy_prxy_bio_serial_schema_field(_name) \
    &field_##_name

#define extern_blgy_prxy_bio_serial_schema_field(_name) \
    extern struct blgy_prxy_bio_serial_schema_field field_##_name;

ssize_t blgy_prxy_bio_serial_schema_serialize(
    struct blgy_prxy_bio_serial_schema_field **schema, char **buf_p
);

ssize_t blgy_prxy_bio_serial_schema_deserialize(
    struct blgy_prxy_bio_serial_schema_field ***schema_p,
    char *buf
);

ssize_t blgy_prxy_bio_serialize(
    struct blgy_prxy_bio_info *bio, char **buf_p,
    struct blgy_prxy_bio_serial_schema_field **schema
);

size_t blgy_prxy_bio_serialized_size(char *buf);

size_t blgy_prxy_bio_deserialize(
    struct blgy_prxy_bio_info *bio, char *buf,
    struct blgy_prxy_bio_serial_schema_field **schema
);

extern_blgy_prxy_bio_serial_schema_field(id);
extern_blgy_prxy_bio_serial_schema_field(start_ts);
extern_blgy_prxy_bio_serial_schema_field(end_ts);
extern_blgy_prxy_bio_serial_schema_field(sector);
extern_blgy_prxy_bio_serial_schema_field(size);
extern_blgy_prxy_bio_serial_schema_field(op);

extern struct blgy_prxy_bio_serial_schema_field *blgy_prxy_bio_serial_schema_default[];

#endif
