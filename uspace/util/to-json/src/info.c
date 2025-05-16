#include "info.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "biology-proxy-bio-serial.h"
#include "json.h"

static ssize_t parse_schema(char *buf,
                     struct blgy_prxy_bio_serial_schema_field ***schema)
{
    return blgy_prxy_bio_serial_schema_deserialize(schema, buf);
}

static ssize_t parse_info(char *buf, struct blgy_prxy_bio_info *info,
                   struct blgy_prxy_bio_serial_schema_field **schema)
{
    return blgy_prxy_bio_deserialize(info, buf, schema);
}

#define REQ_OP_MASK	((1 << 8) - 1)

int parse_dump_file(char *path, int *first)
{
    int ret = 0;
    size_t size;

    int fd = open(path, O_RDWR);
    if (!fd) {
        ret = -EINVAL;
        goto end;
    }

    struct stat stat;
    fstat(fd, &stat);

    char *buf = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    size_t buf_offset = 0;

    struct blgy_prxy_bio_serial_schema_field **schema;
    size = parse_schema(buf + buf_offset, &schema);
    if (size < 0) {
        ret = size;
        goto end;
    }
    buf_offset += size;

    struct blgy_prxy_bio_info info;
    while (buf_offset != stat.st_size) {
        size = parse_info(buf + buf_offset, &info, schema);
        if (size < 0) {
            ret = size;
            goto end;
        }
        buf_offset += size;

        info.op &= REQ_OP_MASK;
        json_bio_info(&info, *first);
        if (*first)
            *first = 0;
    }

end:
    close(fd);
    return ret;
}
