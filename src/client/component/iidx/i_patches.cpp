#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

#include <winhttp.h>

namespace iidx::patches
{
	char asio_name[0x2048];

	const char* __fastcall get_service_url(void* _this, bool is_dev, bool is_kr)
	{
		static auto service_url = ([]
			{
				return game::environment::get_param("LAOCHAN_SERVER_URL");
			}
		)();

		return service_url.data();
	}

	utils::hook::detour init_superstep_sound_hook;
	uint64_t __fastcall init_superstep_sound_stub(void* _this, int /* use_asio */, int sample_rate, int16_t bitrate_device, uint32_t bitrate, int channels, int exclusive)
	{
		return init_superstep_sound_hook.invoke<uint64_t>(
			_this, true,
			sample_rate, bitrate_device, bitrate, channels, exclusive
		);
	}

	bool is_signature_valid()
	{
		return true;
	}

	// bypass ICP registration
	utils::hook::detour on_http_connect_hook;
	HINTERNET WINAPI on_http_connect(HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, DWORD dwReserved)
	{
		auto connect = on_http_connect_hook.invoke_pascal<HINTERNET>(hSession, pswzServerName, nServerPort, dwReserved);

		if (!StrStrW(pswzServerName, L"cloudfront.net"))
		{
			*(reinterpret_cast<uint32_t*>(connect) + 2) = 0x12345678;
		}

		return connect;
	}

	utils::hook::detour on_http_open_request_hook;
	HINTERNET WINAPI on_http_open_request(HINTERNET hConnect, LPCWSTR pwszVerb, LPCWSTR pwszObjectName, LPCWSTR pwszVersion, LPCWSTR pwszReferrer, LPCWSTR* ppwszAcceptTypes, DWORD dwFlags)
	{
		auto request = on_http_open_request_hook.invoke<HINTERNET>(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);

		if (*(reinterpret_cast<uint32_t*>(hConnect) + 2) == 0x12345678)
		{
			*(reinterpret_cast<uint32_t*>(hConnect) + 2) = 0xDDDDDDDD;

			// ignore cert error
			DWORD flags =
				SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
				SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

			WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));

			// overwrite host
			WinHttpAddRequestHeaders(request,
				L"Host: tgk-wcaime.wahlap.com",
				static_cast<DWORD>(-1),
				WINHTTP_ADDREQ_FLAG_REPLACE | WINHTTP_ADDREQ_FLAG_ADD
			);
		}

		return request;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			// disable signature check
			utils::hook::jump(0x140312B60, is_signature_valid);

			// disable music list checksum check
			utils::hook::nop(0x1403017E2, 2);
			utils::hook::nop(0x1403017EE, 2);

			//// change server url
			utils::hook::jump(0x1402F88B0, get_service_url);
			printf("Using bootstrap url: %s\n", get_service_url(nullptr, false, false));

			// override asio device name
			if (game::environment::get_param("IIDX_SOUND_MODE") == "1")
			{
				auto device_name = game::environment::get_param("IIDX_ASIO_DEVICE");
				if (device_name.size() > 0x2047)
					throw std::exception("ASIO Device Name is too long!");

				std::memcpy(asio_name, device_name.data(), device_name.size());

				utils::hook::inject(0x140253B24, asio_name);
				printf("I:launcher: using asio device: %s\n", asio_name);

				// enable retry logic for asio
				utils::hook::nop(0x1401DC897, 6);
				init_superstep_sound_hook.create(0x140253840, init_superstep_sound_stub);
			}

			utils::nt::library winhttp{ "winhttp.dll" };
			on_http_connect_hook.create(winhttp.get_proc<void*>("WinHttpConnect"), on_http_connect);
			on_http_open_request_hook.create(winhttp.get_proc<void*>("WinHttpOpenRequest"), on_http_open_request);
		}
	};
}

REGISTER_COMPONENT(iidx::patches::component, launcher::game::iidx)
