namespace avs2
{
	enum node_type
	{
		NODE_TYPE_node = 1,
		NODE_TYPE_s8 = 2,
		NODE_TYPE_u8 = 3,
		NODE_TYPE_s16 = 4,
		NODE_TYPE_u16 = 5,
		NODE_TYPE_s32 = 6,
		NODE_TYPE_u32 = 7,
		NODE_TYPE_s64 = 8,
		NODE_TYPE_u64 = 9,
		NODE_TYPE_bin = 10,
		NODE_TYPE_str = 11,
		NODE_TYPE_ip4 = 12,
		NODE_TYPE_time = 13,
		NODE_TYPE_float = 14,
		NODE_TYPE_double = 15,
		NODE_TYPE_2s8 = 16,
		NODE_TYPE_2u8 = 17,
		NODE_TYPE_2s16 = 18,
		NODE_TYPE_2u16 = 19,
		NODE_TYPE_2s32 = 20,
		NODE_TYPE_2u32 = 21,
		NODE_TYPE_2s64 = 22,
		NODE_TYPE_2u64 = 23,
		NODE_TYPE_2f = 24,
		NODE_TYPE_2d = 25,
		NODE_TYPE_3s8 = 26,
		NODE_TYPE_3u8 = 27,
		NODE_TYPE_3s16 = 28,
		NODE_TYPE_3u16 = 29,
		NODE_TYPE_3s32 = 30,
		NODE_TYPE_3u32 = 31,
		NODE_TYPE_3s64 = 32,
		NODE_TYPE_3u64 = 33,
		NODE_TYPE_3f = 34,
		NODE_TYPE_3d = 35,
		NODE_TYPE_4s8 = 36,
		NODE_TYPE_4u8 = 37,
		NODE_TYPE_4s16 = 38,
		NODE_TYPE_4u16 = 39,
		NODE_TYPE_4s32 = 40,
		NODE_TYPE_4u32 = 41,
		NODE_TYPE_4s64 = 42,
		NODE_TYPE_4u64 = 43,
		NODE_TYPE_4f = 44,
		NODE_TYPE_4d = 45,
		NODE_TYPE_attr = 46,
		NODE_TYPE_attr_and_node = 47,
		NODE_TYPE_vs8 = 48,
		NODE_TYPE_vu8 = 49,
		NODE_TYPE_vs16 = 50,
		NODE_TYPE_vu16 = 51,
		NODE_TYPE_bool = 52,
		NODE_TYPE_2b = 53,
		NODE_TYPE_3b = 54,
		NODE_TYPE_4b = 55,
		NODE_TYPE_vb = 56,
	};

	struct stat
	{
		uint64_t st_atime;
		uint64_t st_mtime;
		uint64_t st_ctime;
		uint32_t unk1;
		uint32_t filesize;
		struct ::stat padding;
	};

	typedef int32_t file_t;

	struct node_stat
	{
		int nodes;
		int data;
		int unk1, unk2, unk3;
	};
	typedef node_stat* node_stat_ptr;

	struct property_info 
	{
		uint8_t dummy[560];
		uint32_t error_code;
		uint32_t has_error;
		uint32_t unk;
		int8_t buffer_offset;
	};
	typedef property_info* property_ptr;

	struct node_info 
	{
		uint8_t dummy[47];
		node_type type;
	};
	typedef node_info* node_ptr;

	struct psmap_data 
	{
		uint8_t type;
		uint8_t flags;
		uint16_t offset;
		uint32_t size;
		const char* path;
		void* xdefault;
	};

	typedef psmap_data* psmap_data_ptr;
}
