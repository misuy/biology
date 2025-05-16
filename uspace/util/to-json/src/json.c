#include "json.h"

#include "bio-info.h"
#include "biology-proxy-bio-serial.h"

void json_bio_info_begin(void)
{
    printf("{\"requests\": [\n");
}

void json_bio_info_end(void)
{
    printf("\n]}");
}

void json_bio_info(struct blgy_prxy_bio_info *info, int first)
{
    if (!first)
        printf(",\n");

    printf("    {\"cpu\": %d, "
            "\"id\": %d, "
            "\"op\": %d, "
            "\"start_ts\": %lld, "
            "\"end_ts\": %lld, "
            "\"sector\": %llu, "
            "\"size\": %u, "
            "\"status\": %u}",
            info->cpu,
            info->id,
            info->op,
            info->start_ts,
            info->end_ts,
            info->sector,
            info->size,
            info->status);
}
