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

struct blgy_prxy_bio_serial_schema_field *blgy_prxy_bio_serial_schema_default[] = {
    declare_blgy_prxy_bio_serial_schema_field(id),
    declare_blgy_prxy_bio_serial_schema_field(start_ts),
    declare_blgy_prxy_bio_serial_schema_field(end_ts),
    declare_blgy_prxy_bio_serial_schema_field(sector),
    declare_blgy_prxy_bio_serial_schema_field(size),
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

static size_t blgy_prxy_bio_serial_schema_size(
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t offset = 0;
    while (schema[offset] != NULL)
        offset++;
    return sizeof(uint32_t) * offset;
}

ssize_t blgy_prxy_bio_serial_schema_serialize(
    struct blgy_prxy_bio_serial_schema_field **schema, char **buf_p
)
{
    size_t offset = 0;
    size_t size = blgy_prxy_bio_serial_schema_size(schema);
    char *buf = MALLOC_OVERRIDE(size + sizeof(uint32_t));
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

    *buf_p = buf;

    return bufp - buf;
}

ssize_t blgy_prxy_bio_serial_schema_deserialize(
    struct blgy_prxy_bio_serial_schema_field ***schema_p,
    char *buf
)
{
    char *bufp = buf;
    uint32_t size, i, id, cnt;
    struct blgy_prxy_bio_serial_schema_field **schema;

    memcpy(&size, bufp, sizeof(uint32_t));
    bufp += sizeof(uint32_t);

    cnt = size / sizeof(uint32_t);
    schema =
        MALLOC_OVERRIDE(sizeof(struct blgy_prxy_bio_serial_schema_field *)
                        * (cnt + 1));

    if (!schema)
        return -ENOMEM;

    for (i = 0; i < cnt; i++) {
        memcpy(&id, bufp, sizeof(uint32_t));
        bufp += sizeof(uint32_t);
        schema[i] = _get_serial_schema_field_by_id(id);
        if (schema[i] == NULL) {
            FREE_OVERRIDE(schema);
            return -EINVAL;
        }
    }

    schema[cnt] = NULL;
    *schema_p = schema;

    return bufp - buf;
}

static size_t blgy_prxy_bio_serial_size(
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
    struct blgy_prxy_bio_info *info, char **buf_p,
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t offset = 0;
    char *buf = MALLOC_OVERRIDE(blgy_prxy_bio_serial_size(info, schema));
    char *buf_ptr = buf;

    if (buf == NULL) {
        LOG_ERR_OVERRIDE("failed to allocate memory for serialized bio");
        return -ENOMEM;
    }

    while (schema[offset] != NULL) {
        schema[offset]->serialize(schema[offset], info, buf_ptr);
        buf_ptr += schema[offset]->size(info);
        offset++;
    }

    *buf_p = buf;

    return buf_ptr - buf;
}

ssize_t blgy_prxy_bio_deserialize(
    struct blgy_prxy_bio_info *info, char *buf,
    struct blgy_prxy_bio_serial_schema_field **schema
)
{
    size_t offset = 0;
    char *bufp = buf;

    while (schema[offset] != NULL) {
        schema[offset]->deserialize(schema[offset], info, bufp);
        bufp += schema[offset]->size(info);
        offset++;
    }

    return bufp - buf;
}
