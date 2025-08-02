#include <std_include.hpp>
#include <utils/nt.hpp>
#include "audio_client.hpp"

audio_client_proxy::audio_client_proxy(IAudioClient* orig) : client_(orig)
{
}

HRESULT __stdcall audio_client_proxy::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;

	auto hr = client_->QueryInterface(riid, ppvObj);
	if (hr == NOERROR) *ppvObj = this;

	return hr;
}

ULONG __stdcall audio_client_proxy::AddRef(void)
{
	return client_->AddRef();
}

ULONG __stdcall audio_client_proxy::Release(void)
{
	auto count = client_->Release();

	if (!count)
	{
		delete this;
	}

	return count;
}

HRESULT __stdcall audio_client_proxy::Initialize__(AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags, REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity, const WAVEFORMATEX* pFormat, LPCGUID AudioSessionGuid)
{
	static const bool can_set_lowlatency_mode = [] {
		utils::nt::library ntdll{ "ntdll.dll" };

		OSVERSIONINFOEXW info;
		ntdll.invoke_pascal<NTSTATUS>("RtlGetVersion", &info);

		return info.dwMajorVersion >= 10;
		}();
	if (ShareMode != AUDCLNT_SHAREMODE_SHARED || !can_set_lowlatency_mode) return -1;
	auto INVALID_STREAM_FLAGS = AUDCLNT_STREAMFLAGS_CROSSPROCESS | AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_NOPERSIST | AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
	StreamFlags &= ~INVALID_STREAM_FLAGS;
	IAudioClient3* client3 = nullptr;
	UINT32 defaultPeriodInFrames;
	UINT32 fundamentalPeriodInFrames;
	UINT32 minPeriodInFrames;
	UINT32 maxPeriodInFrames;
	auto hr = client_->QueryInterface(__uuidof(IAudioClient3), reinterpret_cast<void**>(&client3));
	if (FAILED(hr)) return hr;
	hr = client3->GetSharedModeEnginePeriod(pFormat, &defaultPeriodInFrames, &fundamentalPeriodInFrames, &minPeriodInFrames, &maxPeriodInFrames);
	if (FAILED(hr)) return hr;
	hr = client3->InitializeSharedAudioStream(StreamFlags, minPeriodInFrames, pFormat, AudioSessionGuid);
	client3->Release();
	return hr;
}

HRESULT __stdcall audio_client_proxy::Initialize(AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags, REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity, const WAVEFORMATEX* pFormat, LPCGUID AudioSessionGuid)
{
	auto hr = audio_client_proxy::Initialize__(ShareMode, StreamFlags, hnsBufferDuration, hnsPeriodicity, pFormat, AudioSessionGuid);
	if (SUCCEEDED(hr))
	{
		return hr;
	}
	else
	{
		return client_->Initialize(ShareMode, StreamFlags, hnsBufferDuration, hnsPeriodicity, pFormat, AudioSessionGuid);
	}
}

HRESULT __stdcall audio_client_proxy::GetBufferSize(UINT32* pNumBufferFrames)
{
	return client_->GetBufferSize(pNumBufferFrames);
}

HRESULT __stdcall audio_client_proxy::GetStreamLatency(REFERENCE_TIME* phnsLatency) 
{
	return client_->GetStreamLatency(phnsLatency);
}

HRESULT __stdcall audio_client_proxy::GetCurrentPadding(UINT32* pNumPaddingFrames)
{
	return client_->GetCurrentPadding(pNumPaddingFrames);
}

HRESULT __stdcall audio_client_proxy::IsFormatSupported(AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch)
{
	return client_->IsFormatSupported(ShareMode, pFormat, ppClosestMatch);
}

HRESULT __stdcall audio_client_proxy::GetMixFormat(WAVEFORMATEX** ppDeviceFormat) 
{
	return client_->GetMixFormat(ppDeviceFormat);
}

HRESULT __stdcall audio_client_proxy::GetDevicePeriod(REFERENCE_TIME* phnsDefaultDevicePeriod, REFERENCE_TIME* phnsMinimumDevicePeriod)
{
	return client_->GetDevicePeriod(phnsDefaultDevicePeriod, phnsMinimumDevicePeriod);
}

HRESULT __stdcall audio_client_proxy::Start(void) 
{
	return client_->Start();
}

HRESULT __stdcall audio_client_proxy::Stop(void) 
{
	return client_->Stop();
}

HRESULT __stdcall audio_client_proxy::Reset(void) 
{
	return client_->Reset();
}

HRESULT __stdcall audio_client_proxy::SetEventHandle(HANDLE eventHandle)
{
	return client_->SetEventHandle(eventHandle);
}

HRESULT __stdcall audio_client_proxy::GetService(REFIID riid, void** ppv) 
{
	return client_->GetService(riid, ppv);
}