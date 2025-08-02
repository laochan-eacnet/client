#pragma once

class mmdevice_enumerator_proxy : public IMMDeviceEnumerator
{
public:
	mmdevice_enumerator_proxy(IMMDeviceEnumerator* orig);

private:
	IMMDeviceEnumerator* enumerator_;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall EnumAudioEndpoints(EDataFlow dataFlow, DWORD dwStateMask, IMMDeviceCollection** ppDevices) override;
	virtual HRESULT __stdcall GetDefaultAudioEndpoint(EDataFlow dataFlow, ERole role, IMMDevice** ppEndpoint) override;
	virtual HRESULT __stdcall GetDevice(LPCWSTR pwstrId, IMMDevice** ppDevice) override;
	virtual HRESULT __stdcall RegisterEndpointNotificationCallback(IMMNotificationClient* pClient) override;
	virtual HRESULT __stdcall UnregisterEndpointNotificationCallback(IMMNotificationClient* pClient) override;
};
