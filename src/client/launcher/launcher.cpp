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

#include "component/steam_proxy.hpp"

#ifndef _DEBUG
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
				.chrome_flags =
				{
					"--disable-gpu-vsync",
					"--disable-frame-rate-limit",
				}
			}
		), [](borderless_smartview* smartview)
		{
			delete smartview;
		}
	);

	this->create_main_menu();
}

void launcher::create_main_menu()
{
	smartview_->set_title("Laochan-Eacnet Launcher");
	smartview_->set_dpi_aware_size(1280, 720);

	smartview_->expose("version", []() -> std::string
		{
			return VERSION;
		}
	);

	smartview_->expose("close", [this]()
		{
			smartview_->send_close();
		}
	);

	smartview_->expose("minimize", [this]()
		{
			smartview_->set_minimized(true);
		}
	);

	smartview_->expose("mounted", [this]()
		{
			smartview_->show();
		}
	);

	smartview_->expose("shellExecute", [](std::string command, std::string args)
		{
			ShellExecuteA(nullptr, "open", command.data(), args.data(), nullptr, 1);
		}, false
	);

	smartview_->expose("detectGameInstall", [](int game_index) -> std::vector<std::string>
		{
			::game::environment::set_game(static_cast<game>(game_index));

			try
			{
				auto install = ::game::environment::get_install_path().string();
				auto resource = ::game::environment::get_resource_path().string();

				return { install, resource };
			}
			catch (std::exception)
			{

			}

			return {};
		}
	);

	smartview_->expose("readFile", [](std::string path) -> std::string
		{
			if (!std::filesystem::exists(path))
				return "<NOT EXIST>";

			FILE* file = nullptr;
			fopen_s(&file, path.data(), "r");
			fseek(file, 0, SEEK_END);
			auto size = ftell(file);
			fseek(file, 0, SEEK_SET);

			std::string buffer;
			buffer.resize(size + 1);
			fread(buffer.data(), 1, size, file);
			buffer.resize(size);

			fclose(file);

			return buffer;
		}, true
	);

	smartview_->expose("writeFile", [](std::string path, std::string content) -> void
		{
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			FILE* file = nullptr;
			fopen_s(&file, path.data(), "w");
			fwrite(content.data(), 1, content.size(), file);
			fclose(file);
		}
	);

	smartview_->expose("uuid", []() -> std::string
		{
			const auto steamid = steam_proxy::get_steam_id();
			if (steamid != 0xFFFFFFFFDEADBEEFul)
				return std::to_string(steamid);

			const auto uuid = utils::string::dump_hex(utils::smbios::get_uuid(), "");
			return uuid;
		}
	);

	smartview_->expose("setGame", [](int target) -> void
		{
			auto result = static_cast<launcher::game>(target);
			if (result >= launcher::game::count || result <= launcher::game::invalid)
				return;

			target_game = result;
		}
	);

	smartview_->expose("setParam", [](std::string key, std::string value) -> void
		{
			::game::environment::set_param(key, value);
		}
	);

	smartview_->expose("getAsioDeviceList", []() -> std::vector<std::string>
		{
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
		}
	);

	smartview_->expose("checkWasapiDeviceStatus", []() -> int
		{
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
		}
	);

#ifdef _DEBUG
	smartview_->set_url("http://localhost:5173/");
	smartview_->set_dev_tools(true);
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
