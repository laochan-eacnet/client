#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <winhttp.h>

namespace icp_bypass
{
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

		if (*(reinterpret_cast<uint32_t*>(hConnect) + 2) != 0x12345678)
			return request;

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

		return request;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library winhttp{ "winhttp.dll" };

			on_http_connect_hook.create(winhttp.get_proc<void*>("WinHttpConnect"), on_http_connect);
			on_http_open_request_hook.create(winhttp.get_proc<void*>("WinHttpOpenRequest"), on_http_open_request);
		}
	};
}

REGISTER_COMPONENT(icp_bypass::component, launcher::game::all)
