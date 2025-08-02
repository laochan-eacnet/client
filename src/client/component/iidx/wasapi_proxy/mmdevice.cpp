#include <std_include.hpp>
#include "mmdevice.hpp"
#include "audio_client.hpp"

mmdevice_proxy::mmdevice_proxy(IMMDevice* orig) : device_(orig)
{
}

HRESULT __stdcall mmdevice_proxy::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;

	auto hr = device_->QueryInterface(riid, ppvObj);
	if (hr == NOERROR) *ppvObj = this;

	return hr;
}

ULONG __stdcall mmdevice_proxy::AddRef(void)
{
	return device_->AddRef();
}

ULONG __stdcall mmdevice_proxy::Release(void)
{
	auto count = device_->Release();

	if (!count)
	{
		delete this;
	}

	return count;
}

HRESULT __stdcall mmdevice_proxy::Activate(REFIID iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface) 
{
	auto hr = device_->Activate(iid, dwClsCtx, pActivationParams, ppInterface);
	if (SUCCEEDED(hr) && IsEqualIID(iid, __uuidof(IAudioClient)))
	{
		*ppInterface = new audio_client_proxy(reinterpret_cast<IAudioClient*>(*ppInterface));
	}
	return hr;
}

HRESULT __stdcall mmdevice_proxy::OpenPropertyStore(DWORD stgmAccess, IPropertyStore** ppProperties) 
{
	return device_->OpenPropertyStore(stgmAccess, ppProperties);
}

HRESULT __stdcall mmdevice_proxy::GetId(LPWSTR* ppstrId) 
{
	return device_->GetId(ppstrId);
}

HRESULT __stdcall mmdevice_proxy::GetState(DWORD* pdwState) 
{
	return device_->GetState(pdwState);
}