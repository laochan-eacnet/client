#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

#include <winhttp.h>

char asio_name[0x2048];

namespace patches
{
	const char* __fastcall get_service_url(void* _this, bool is_dev, bool is_kr)
	{
		static auto service_url = ([]
			{
				return game::environment::get_param("LAOCHAN_SERVER_URL");
			}
		)();

		return service_url.data();
	}

	//utils::hook::detour init_superstep_sound_hook;
	//uint64_t __fastcall init_superstep_sound_stub(void* a1, int /* use_asio */, int a3, int16_t a4, uint32_t a5, int a6, int a7)
	//{
	//	return init_superstep_sound_hook.invoke<uint64_t, void*, int, int, int16_t, uint32_t, int, int>(
	//		a1, launcher::snd_mode == launcher::sound_mode::asio ? 1 : 0,
	//		a3, a4, a5, a6, a7
	//	);
	//}

	bool is_signature_valid()
	{
		return true;
	}

	// bypass ICP registration
	utils::hook::detour on_http_request_hook;
	bool on_http_request(HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength,
		LPVOID lpOptional, DWORD dwOptionalLength, DWORD dwTotalLength, DWORD_PTR dwContext
	)
	{
		// ignore cert error
		DWORD dwFlags =
			SECURITY_FLAG_IGNORE_UNKNOWN_CA |
			SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
			SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
			SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

		WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

		// overwrite host
		/*WinHttpAddRequestHeaders(hRequest,
			L"Host: laochan.wahlap.com",
			static_cast<DWORD>(-1),
			WINHTTP_ADDREQ_FLAG_REPLACE | WINHTTP_ADDREQ_FLAG_ADD
		);*/

		return on_http_request_hook.get<decltype(WinHttpSendRequest)>()
			(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength, dwTotalLength, dwContext);
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			return;

			// disable signature check
			utils::hook::jump(0x140312B60, is_signature_valid);

			// disable music list checksum check
			utils::hook::nop(0x1403017E2, 2);
			utils::hook::nop(0x1403017EE, 2);

			//// change server url
			utils::hook::jump(0x1402F88B0, get_service_url);
			printf("Using bootstrap url: %s\n", get_service_url(nullptr, false, false));

			//// override asio device name
			//if (launcher::asio_device_name.size() > 0x2047)
			//	throw std::exception("ASIO Device Name is too long!");

			//std::memcpy(asio_name, launcher::asio_device_name.data(), launcher::asio_device_name.size());

			//utils::hook::inject(0x1402539C4, asio_name);
			//printf("I:launcher: using asio device: %s\n", launcher::asio_device_name.data());

			// enable retry logic for asio
			utils::hook::nop(0x1401DC737, 6);
			// init_superstep_sound_hook.create(0x1402536E0, init_superstep_sound_stub);

			utils::nt::library winhttp{ "winhttp.dll" };
			on_http_request_hook.create(winhttp.get_proc<void*>("WinHttpSendRequest"), on_http_request);
		}
	};
}

REGISTER_COMPONENT(patches::component, launcher::game::iidx)
