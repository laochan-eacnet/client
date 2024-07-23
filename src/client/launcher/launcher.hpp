#pragma once
#include "launcher/launcher_module.hpp"

using borderless_smartview = saucer::smartview<saucer::default_serializer, launcher_module>;
using borderless_smartview_ptr = std::unique_ptr<borderless_smartview, std::function<void(borderless_smartview*)>>;

class launcher final
{
public:
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

	game run() const;
private:
	void create_main_menu();
	borderless_smartview_ptr smartview_;
};
