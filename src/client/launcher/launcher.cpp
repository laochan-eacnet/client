#include <std_include.hpp>

#include "launcher.hpp"
#include "launcher_module.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <steam/steam.hpp>
#include <game/game.hpp>

#include "mmdeviceapi.h"
#include "audioclient.h"

#include "dxgi.h"

#include "component/steam_proxy.hpp"

#define USE_EMBEDDED 1

#if !_DEBUG || USE_EMBEDDED
#include "resources/all.hpp"
#endif

namespace
{
	launcher::game target_game = launcher::game::invalid;
}

launcher::launcher()
{
	smartview_ = borderless_smartview_ptr(
		new borderless_smartview(
			saucer::options
			{
				.storage_path = ".\\user",
				.chrome_flags =
				{
				}
			}
		), [](borderless_smartview* smartview) {
		delete smartview;
	});

	this->create_main_menu();
}

void launcher::create_main_menu()
{
	smartview_->set_title("Laochan-Eacnet Launcher");
	smartview_->set_dpi_aware_size(1280, 720);

	smartview_->expose("version", []() -> std::string {
		return VERSION;
	});

	smartview_->expose("close", [this]() {
		smartview_->send_close();
	});

	smartview_->expose("minimize", [this]() {
		smartview_->set_minimized(true);
	});

	smartview_->expose("mounted", [this]() {
		smartview_->show();
	});

	smartview_->expose("shellExecute", [](std::string command, std::string args) {
		ShellExecuteA(nullptr, "open", command.data(), args.data(), nullptr, 1);
	}, false);

	smartview_->expose("detectGameInstall", [](int game_index) -> std::string {
		try
		{
			auto g = static_cast<launcher::game>(game_index);

			if (g >= launcher::game::count || g <= launcher::game::invalid)
			{
				MessageBoxA(nullptr, "game index out of range", "ERROR", MB_ICONERROR);
				return {};
			}

			auto meta = ::game::environment::gamemeta::get_gamemeta(g);

			nlohmann::json result
			{
				{ "game_type", game_index },
				{ "game_name", ::game::environment::get_string(static_cast<launcher::game>(game_index)) },
				{ "installed", meta.get_install_state() },
			};

			if (meta.get_install_state())
			{
				result["install_path"] = utils::string::wide_to_utf8(meta.get_install_path());
				result["resource_path"] = utils::string::wide_to_utf8(meta.get_resource_path());
				result["game_module_path"] = utils::string::wide_to_utf8(meta.get_game_module_path());
				result["settings_module_path"] = utils::string::wide_to_utf8(meta.get_settings_module_path());
				result["updater_module_path"] = utils::string::wide_to_utf8(meta.get_updater_module_path());
				result["game_module_version"] = meta.get_game_module_version();
				result["game_module_target_version"] = meta.get_game_module_target_version();
			}

			return result.dump();
		}
		catch (std::exception&)
		{
		}

		return {};
	}, true);

	smartview_->expose("readFile", [](std::string path) -> std::string {
		if (!std::filesystem::exists(path))
			return "<NOT EXIST>";
		auto pathw = utils::string::utf8_to_wide(path);
		std::ifstream file_stream{ pathw, std::ios::binary | std::ios::ate };

		if (!file_stream.is_open())
			return "<NOT EXISTS>";

		auto size = file_stream.tellg();
		file_stream.seekg(0, std::ios::beg);

		std::string buffer;
		buffer.resize(size);
		file_stream.read(buffer.data(), size);

		return buffer;
	}, true);

	smartview_->expose("writeFile", [](std::string path, std::string content) -> void {
		if (std::filesystem::exists(path))
			std::filesystem::remove(path);

		auto pathw = utils::string::utf8_to_wide(path);
		std::ofstream file_stream{ pathw, std::ios::binary };

		if (!file_stream.is_open())
		{
			printf("W:Failed to write %llu bytes to %s!\n", content.size(), path.data());
			return;
		}

		file_stream.write(content.data(), content.size());
	});

	smartview_->expose("uuid", []() -> std::string {
		const auto steamid = steam_proxy::get_steam_id();
		if (steamid != 0xFFFFFFFFDEADBEEFul)
			return std::to_string(steamid);

		const auto uuid = utils::string::dump_hex(utils::smbios::get_uuid(), "");
		return uuid;
	});

	smartview_->expose("setGame", [](int target) -> void {
		auto result = static_cast<launcher::game>(target);
		if (result >= launcher::game::count || result <= launcher::game::invalid)
			return;

		target_game = result;
	});

	smartview_->expose("setParam", [](std::string key, std::string value) -> void {
		::game::environment::set_param(key, value);
	});

	smartview_->expose("getAsioDeviceList", []() -> std::vector<std::string> {
		std::vector<std::string> result;

		HKEY key;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\ASIO", 0, KEY_READ, &key))
			return result;

		DWORD subkey_counts, subkey_name_maxlen;
		RegQueryInfoKeyA(key, nullptr, nullptr, nullptr, &subkey_counts, &subkey_name_maxlen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

		char* buffer = utils::memory::allocate<char>(subkey_name_maxlen * 2);

		for (DWORD i = 0; i < subkey_counts; i++)
		{
			DWORD size = subkey_name_maxlen * 2;
			RegEnumKeyExA(key, i, buffer, &size, nullptr, nullptr, nullptr, nullptr);

			result.push_back(std::string{ buffer });
		}

		utils::memory::free(buffer);
		return result;
	});

	smartview_->expose("checkWasapiDeviceStatus", []() -> int {
		IMMDeviceEnumerator* device_enumerator = nullptr;

		auto hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&device_enumerator));
		if (FAILED(hr))
		{
			return -1;
		}

		IMMDevice* device = nullptr;
		hr = device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

		if (FAILED(hr))
		{
			device_enumerator->Release();
			return -2;
		}

		IAudioClient* audio_client = nullptr;
		hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&audio_client));

		if (FAILED(hr))
		{
			device->Release();
			device_enumerator->Release();
			return -3;
		}

		WAVEFORMATEXTENSIBLE wf = { 0 };
		wf.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wf.Format.nChannels = 2;
		wf.Format.nSamplesPerSec = 44100;
		wf.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;;
		wf.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

		const WORD BIT_SETS[][2] =
		{
			{ 32, 32 },
			{ 24, 24 },
			{ 16, 24 },
			{ 16, 16 },
		};

		for (auto& bit_set : BIT_SETS)
		{
			wf.Format.wBitsPerSample = bit_set[0];
			wf.Samples.wValidBitsPerSample = bit_set[0];
			wf.Format.nBlockAlign = wf.Format.nChannels * wf.Format.wBitsPerSample / 8;
			wf.Format.nAvgBytesPerSec = wf.Format.nSamplesPerSec * wf.Format.nBlockAlign;

			hr = audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, reinterpret_cast<WAVEFORMATEX*>(&wf), nullptr);
			if (SUCCEEDED(hr))
			{
				audio_client->Release();
				device->Release();
				device_enumerator->Release();

				return 0;
			}
		}

		audio_client->Release();
		device->Release();
		device_enumerator->Release();
		return 1;
	});

	smartview_->expose("queryDisplayModes", []() -> std::vector<std::string> {
		static std::vector<std::string> result;
		if (result.size())
			return result;

		IDXGIFactory* factory = nullptr;
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

		IDXGIAdapter* adapter = nullptr;
		factory->EnumAdapters(0, &adapter);

		IDXGIOutput* output = nullptr;
		adapter->EnumOutputs(0, &output);

		UINT mode_count = 0;
		output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, nullptr);

		auto modes = utils::memory::allocate_array<DXGI_MODE_DESC>(mode_count);
		output->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, 0, &mode_count, modes);

		for (UINT i = 0; i < mode_count; i++)
		{
			auto mode = modes[i];
			auto mode_formatted = utils::string::va("[%d,%d,%.2f]", mode.Width, mode.Height, mode.RefreshRate.Numerator / static_cast<float>(mode.RefreshRate.Denominator));

			if (std::find(result.begin(), result.end(), mode_formatted) == result.end())
				result.push_back(mode_formatted);
		}

		utils::memory::free(modes);

		output->Release();
		adapter->Release();
		factory->Release();

		return result;
	});

	smartview_->expose("selfPath", []() -> std::string {
		utils::nt::library self{};
		return utils::string::wide_to_utf8(self.get_path().generic_wstring());
	});

#if _DEBUG
	smartview_->set_dev_tools(true);
#endif

#if !USE_EMBEDDED
	smartview_->set_url("http://localhost:5173/");
#else
	smartview_->embed(laochan::embedded::all());
	smartview_->serve("");
#endif
}

launcher::game launcher::run() const
{
	smartview_->run();
	return target_game;
}
