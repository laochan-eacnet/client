#pragma once

class launcher final
{
public:
	enum class display_mode: uint32_t
	{
		fullscreen,
		windowed_720p,
		windowed_900p,
		windowed_1080p,
	};

	enum class sound_mode : uint32_t
	{
		wasapi,
		asio,
	};

	enum class game : int32_t
	{
		invalid = -1,

		iidx,
		sdvx,
		gitadora,
		
		count,
		all = count,
	};

	launcher();

	bool run() const;
private:
	void create_main_menu();
	static std::string load_content(int res);
};
