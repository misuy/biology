#include "info.h"
#include "biology-proxy-bio-serial.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

int parse_dump_file(char *path)
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
    printf("size %lu\n", stat.st_size);

    char *buf = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    size_t buf_offset = 0;

    struct blgy_prxy_bio_serial_schema_field **schema;
    size = parse_schema(buf + buf_offset, &schema);
    if (size < 0) {
        ret = size;
        goto end;
    }
    buf_offset += size;

    int offset = 0;
    while (schema[offset] != 0) {
        printf("schema %d id: %u\n", offset, schema[0]->id);
        offset++;
    }

    struct blgy_prxy_bio_info info;
    while (buf_offset != stat.st_size) {
        size = parse_info(buf + buf_offset, &info, schema);
        if (size < 0) {
            ret = size;
            goto end;
        }
        buf_offset += size;
        printf("id: %u, size: %u, start_ts: %lld, end_ts: %lld, sector: %llu\n", info.id, info.size, info.start_ts, info.end_ts, info.sector);
    }

end:
    close(fd);
    return ret;
}
