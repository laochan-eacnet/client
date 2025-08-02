#include <std_include.hpp>
#include "mmdevice_enumerator.hpp"
#include "mmdevice_collection.hpp"
#include "mmdevice.hpp"

mmdevice_enumerator_proxy::mmdevice_enumerator_proxy(IMMDeviceEnumerator* orig) : enumerator_(orig)
{
}

HRESULT __stdcall mmdevice_enumerator_proxy::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;

	auto hr = enumerator_->QueryInterface(riid, ppvObj);
	if (hr == NOERROR) *ppvObj = this;

	return hr;
}

ULONG __stdcall mmdevice_enumerator_proxy::AddRef(void)
{
	return enumerator_->AddRef();
}

ULONG __stdcall mmdevice_enumerator_proxy::Release(void)
{
	auto count = enumerator_->Release();

	if (!count)
	{
		delete this;
	}

	return count;
}

HRESULT __stdcall mmdevice_enumerator_proxy::EnumAudioEndpoints(EDataFlow dataFlow, DWORD dwStateMask, IMMDeviceCollection** ppDevices)
{
	auto hr = enumerator_->EnumAudioEndpoints(dataFlow, dwStateMask, ppDevices);
	if (SUCCEEDED(hr))
	{
		*ppDevices = new mmdevice_collection_proxy(*ppDevices);
	}
	return hr;
}

HRESULT __stdcall mmdevice_enumerator_proxy::GetDefaultAudioEndpoint(EDataFlow dataFlow, ERole role, IMMDevice** ppEndpoint)
{
	auto hr = enumerator_->GetDefaultAudioEndpoint(dataFlow, role, ppEndpoint);
	if (SUCCEEDED(hr))
	{
		*ppEndpoint = new mmdevice_proxy(*ppEndpoint);
	}
	return hr;
}

HRESULT __stdcall mmdevice_enumerator_proxy::GetDevice(LPCWSTR pwstrId, IMMDevice** ppDevice)
{
	auto hr = enumerator_->GetDevice(pwstrId, ppDevice);
	if (SUCCEEDED(hr))
	{
		*ppDevice = new mmdevice_proxy(*ppDevice);
	}
	return hr;
}

HRESULT __stdcall mmdevice_enumerator_proxy::RegisterEndpointNotificationCallback(IMMNotificationClient* pClient)
{
	return enumerator_->RegisterEndpointNotificationCallback(pClient);
}

HRESULT __stdcall mmdevice_enumerator_proxy::UnregisterEndpointNotificationCallback(IMMNotificationClient* pClient)
{
	return enumerator_->UnregisterEndpointNotificationCallback(pClient);
}