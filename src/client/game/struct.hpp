#pragma once
namespace game
{
    struct avs_stat {
        uint64_t st_atime;
        uint64_t st_mtime;
        uint64_t st_ctime;
        uint32_t unk1;
        uint32_t filesize;
        struct stat padding;
    };
}
