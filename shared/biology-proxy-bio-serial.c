#include "biology-proxy-bio-serial.h"

define_blgy_prxy_bio_serial_schema_field(
    0, id, id,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    1, start_ts, start_ts,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    2, end_ts, end_ts,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    3, sector, sector,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    4, size, size,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    5, op, op,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    6, cpu, cpu,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

define_blgy_prxy_bio_serial_schema_field(
    7, status, status,
    blgy_prxy_bio_serial_schema_field_size_fn_default,
    blgy_prxy_bio_serial_schema_field_ptr_fn_default,
    blgy_prxy_bio_serial_schema_field_serialize_fn_default,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_default
);

#define blgy_prxy_bio_serial_schema_field_size_fn_payload(_field) \
    return info->_field.size + sizeof(info->_field.size);

#ifdef C_PRXY

#define blgy_prxy_bio_serial_schema_field_serialize_fn_payload(_field)      \
    struct bio *bio = info->_field.data;                                    \
    struct bio_vec *bvec = NULL;                                            \
    int i = 0;                                                              \
    size_t offset = 0;                                                      \
                                                                            \
    memcpy(buf + offset, &info->_field.size, sizeof(info->_field.size));    \
    offset += sizeof(info->_field.size);                                    \
                                                                            \
    if (info->_field.size == 0)                                             \
        return;                                                             \
                                                                            \
    for (i = 0; i < bio->bi_vcnt; i++) {                                    \
        bvec = bio->bi_io_vec + i;                                          \
        memcpy(buf + offset, bvec->bv_page + bvec->bv_offset, bvec->bv_len);\
        offset += bvec->bv_len;                                             \
    }                                                                       \

#define blgy_prxy_bio_serial_schema_field_deserialize_fn_payload(_field)    \
    blgy_prxy_bio_serial_schema_field_deserialize_fn_stub(_field)

#else

#define blgy_prxy_bio_serial_schema_field_serialize_fn_payload(_field)      \
    blgy_prxy_bio_serial_schema_field_serialize_fn_stub(_field)

#define blgy_prxy_bio_serial_schema_field_deserialize_fn_payload(_field)    \
    memcpy(&info->_field.size, buf, sizeof(info->_field.size));             \
    if (info->_field.size == 0) {                                           \
        info->_field.data = NULL;                                           \
        return;                                                             \
    }                                                                       \
    info->_field.data = MALLOC_OVERRIDE(info->_field.size);                 \
    memcpy(info->_field.data, buf + sizeof(info->_field.size),              \
           info->_field.size);                                              \

#endif

define_blgy_prxy_bio_serial_schema_field(
    8, payload, payload,
    blgy_prxy_bio_serial_schema_field_size_fn_payload,
    blgy_prxy_bio_serial_schema_field_ptr_fn_stub,
    blgy_prxy_bio_serial_schema_field_serialize_fn_payload,
    blgy_prxy_bio_serial_schema_field_deserialize_fn_payload
);

struct blgy_prxy_bio_serial_schema_field *blgy_prxy_bio_serial_schema_default[] = {
    declare_blgy_prxy_bio_serial_schema_field(id),
    declare_blgy_prxy_bio_serial_schema_field(start_ts),
    declare_blgy_prxy_bio_serial_schema_field(end_ts),
    declare_blgy_prxy_bio_serial_schema_field(sector),
    declare_blgy_prxy_bio_serial_schema_field(size),
    declare_blgy_prxy_bio_serial_schema_field(op),
    declare_blgy_prxy_bio_serial_schema_field(cpu),
    declare_blgy_prxy_bio_serial_schema_field(status),
    NULL
};

struct blgy_prxy_bio_serial_schema_field *blgy_prxy_bio_serial_schema_payload[] = {
    declare_blgy_prxy_bio_serial_schema_field(id),
    declare_blgy_prxy_bio_serial_schema_field(start_ts),
    declare_blgy_prxy_bio_serial_schema_field(end_ts),
    declare_blgy_prxy_bio_serial_schema_field(sector),
    declare_blgy_prxy_bio_serial_schema_field(size),
    declare_blgy_prxy_bio_serial_schema_field(op),
    declare_blgy_prxy_bio_serial_schema_field(cpu),
    declare_blgy_prxy_bio_serial_schema_field(status),
    declare_blgy_prxy_bio_serial_schema_field(payload),
    NULL
};

static struct blgy_prxy_bio_serial_schema_field *
_get_serial_schema_field_by_id(uint32_t id)
{
    size_t offset = 0;
    while (blgy_prxy_bio_serial_schema_default[offset] != NULL) {
        if (blgy_prxy_bio_serial_schema_default[offset]->id == id)
            return blgy_prxy_bio_serial_schema_default[offset];

        offset++;
    }
    return NULL;
}

static uint32_t blgy_prxy_bio_serial_schema_size(
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t offset = 0;
    while (schema[offset] != NULL)
        offset++;
    return offset;
}

ssize_t blgy_prxy_bio_serial_schema_serialize(
    struct blgy_prxy_bio_serial_schema_field **schema, char *buf
)
{
    size_t offset = 0;
    uint32_t size = blgy_prxy_bio_serial_schema_size(schema);
    char *bufp = buf;

    if (!buf)
        return -ENOMEM;

    memcpy(bufp, &size, sizeof(uint32_t));
    bufp += sizeof(uint32_t);

    while (schema[offset] != NULL) {
        memcpy(bufp, &schema[offset]->id, sizeof(uint32_t));
        bufp += sizeof(uint32_t);
        offset++;
    }

    return bufp - buf;
}

ssize_t blgy_prxy_bio_serial_schema_deserialize(
    struct blgy_prxy_bio_serial_schema_field ***schema_p,
    char *buf
)
{
    char *bufp = buf;
    uint32_t size, i, id;
    struct blgy_prxy_bio_serial_schema_field **schema;

    memcpy(&size, bufp, sizeof(uint32_t));
    bufp += sizeof(uint32_t);

    schema =
        MALLOC_OVERRIDE(sizeof(struct blgy_prxy_bio_serial_schema_field *)
                        * (size + 1));

    if (!schema)
        return -ENOMEM;

    for (i = 0; i < size; i++) {
        memcpy(&id, bufp, sizeof(uint32_t));
        bufp += sizeof(uint32_t);
        schema[i] = _get_serial_schema_field_by_id(id);
        if (schema[i] == NULL) {
            FREE_OVERRIDE(schema);
            return -EINVAL;
        }
    }

    schema[size] = NULL;
    *schema_p = schema;

    return bufp - buf;
}

size_t blgy_prxy_bio_serial_size(
    struct blgy_prxy_bio_info *info,
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t size = 0, offset = 0;
    while (schema[offset] != NULL) {
        size += schema[offset]->size(info);
        offset++;
    }
    return size;
}

ssize_t blgy_prxy_bio_serialize(
    struct blgy_prxy_bio_info *info, char *buf,
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t offset = 0, size = blgy_prxy_bio_serial_size(info, schema);
    char *bufp = buf;

    memcpy(bufp, &size, sizeof(size_t));
    bufp += sizeof(size_t);

    while (schema[offset] != NULL) {
        schema[offset]->serialize(schema[offset], info, bufp);
        bufp += schema[offset]->size(info);
        offset++;
    }

    return bufp - buf;
}

size_t blgy_prxy_bio_serialized_size(char *buf)
{
    size_t size;
    memcpy(&size, buf, sizeof(size_t));
    return size + sizeof(size_t);
}

size_t blgy_prxy_bio_deserialize(
    struct blgy_prxy_bio_info *info, char *buf,
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t offset = 0;
    char *bufp = buf;

    bufp += sizeof(size_t);

    while (schema[offset] != NULL) {
        schema[offset]->deserialize(schema[offset], info, bufp);
        bufp += schema[offset]->size(info);
        offset++;
    }

    return bufp - buf;
}
