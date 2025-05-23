#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "filesystem.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace shiftjis
{
	NTSTATUS WINAPI mb_to_unicode_n(PWCH out_wstr, ULONG out_size, PULONG result_size, const CHAR* in_str, ULONG in_size)
	{
		const auto size = MultiByteToWideChar(932, 0, in_str, in_size, out_wstr, out_size);
		if (result_size) *result_size = (size + 1) * sizeof(wchar_t);
		return 0;
	}

	NTSTATUS WINAPI mb_to_unicode_size(PULONG result_size, const CHAR* in_str, ULONG in_size)
	{
		*result_size = (MultiByteToWideChar(932, 0, in_str, in_size, nullptr, 0) + 1) * sizeof(wchar_t);
		return 0;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library ntdll{ "ntdll.dll" };

			utils::hook::jump(ntdll.get_proc<size_t>("RtlMultiByteToUnicodeN"), mb_to_unicode_n, true);
			utils::hook::jump(ntdll.get_proc<size_t>("RtlMultiByteToUnicodeSize"), mb_to_unicode_size, true);
		}
	};
}

REGISTER_COMPONENT(shiftjis::component, launcher::game::all)
