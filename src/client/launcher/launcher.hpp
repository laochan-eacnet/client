#pragma once
#include "launcher/launcher_module.hpp"

using borderless_smartview = saucer::smartview<saucer::default_serializer, launcher_module>;
using borderless_smartview_ptr = std::unique_ptr<borderless_smartview, std::function<void(borderless_smartview*)>>;

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

	borderless_smartview_ptr smartview_;
};
