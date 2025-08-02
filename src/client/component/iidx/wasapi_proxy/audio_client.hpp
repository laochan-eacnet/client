#pragma once

class audio_client_proxy : public IAudioClient
{
public:
	audio_client_proxy(IAudioClient* orig);

private:
	IAudioClient* client_;
	HRESULT __stdcall Initialize__(AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags, REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity, const WAVEFORMATEX* pFormat, LPCGUID AudioSessionGuid);
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall Initialize(AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags, REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity, const WAVEFORMATEX* pFormat, LPCGUID AudioSessionGuid) override;
	virtual HRESULT __stdcall GetBufferSize(UINT32* pNumBufferFrames) override;
	virtual HRESULT __stdcall GetStreamLatency(REFERENCE_TIME* phnsLatency) override;
	virtual HRESULT __stdcall GetCurrentPadding(UINT32* pNumPaddingFrames) override;
	virtual HRESULT __stdcall IsFormatSupported(AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch) override;
	virtual HRESULT __stdcall GetMixFormat(WAVEFORMATEX** ppDeviceFormat) override;
	virtual HRESULT __stdcall GetDevicePeriod(REFERENCE_TIME* phnsDefaultDevicePeriod, REFERENCE_TIME* phnsMinimumDevicePeriod) override;
	virtual HRESULT __stdcall Start(void) override;
	virtual HRESULT __stdcall Stop(void) override;
	virtual HRESULT __stdcall Reset(void) override;
	virtual HRESULT __stdcall SetEventHandle(HANDLE eventHandle) override;
	virtual HRESULT __stdcall GetService(REFIID riid, void** ppv) override;
};