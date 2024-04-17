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

#define CUR_STYLE_ENTRIES (80 * 1000)
#define MAX_ENTRIES (CUR_STYLE_ENTRIES + 1000)

#pragma pack(push, 1)
    struct bpm_t {
        uint32_t min;
        uint32_t max;
    };

    struct music_t {
        char title[0x40];
        char title_ascii[0x40];
        char genre[0x40];
        char artist[0x40];

        uint32_t texture_title;
        uint32_t texture_artist;
        uint32_t texture_genre;
        uint32_t texture_load;
        uint32_t texture_list;

        uint32_t font_idx;
        uint16_t game_version;

        uint16_t other_folder;
        uint16_t bemani_folder;
        uint16_t splittable_diff;

        uint8_t level_sp[5];
        uint8_t level_dp[5];

        uint8_t __gap1[2];

        bpm_t bpm[10];

        uint8_t __gap2[52];

        uint32_t note_count[10];

        uint8_t __gap3[88];
        uint32_t chart_type[10];
        uint8_t __gap4[24];

        uint32_t song_id;
        uint32_t volume;

        uint8_t ident_sp[5];
        uint8_t ident_dp[5];

        uint16_t bga_delay;

        uint16_t __gap5;

        char bga_filename[0x20];

        uint16_t __gap6;

        uint32_t afp_flag;

        uint8_t afp_data[0x20 * 10];
        uint32_t __gap7;
    };
    
    static_assert(offsetof(music_t, song_id) == 624);
    static_assert(offsetof(music_t, bga_filename) == 646);
    static_assert(sizeof(music_t) == 0x3F0);

    struct music_data_t {
        uint32_t header;
        uint32_t data_version;
        uint16_t music_count;
        uint16_t zero;
        uint32_t max_entries;
        int16_t index_table[MAX_ENTRIES];
        music_t musics[1];
    };

#pragma pack(pop)
}
