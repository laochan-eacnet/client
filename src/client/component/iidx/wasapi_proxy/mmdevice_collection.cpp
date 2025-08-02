#include <std_include.hpp>
#include "mmdevice_collection.hpp"
#include "mmdevice.hpp"

mmdevice_collection_proxy::mmdevice_collection_proxy(IMMDeviceCollection* orig) : collection_(orig)
{
}

HRESULT __stdcall mmdevice_collection_proxy::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;

	auto hr = collection_->QueryInterface(riid, ppvObj);
	if (hr == NOERROR) *ppvObj = this;

	return hr;
}

ULONG __stdcall mmdevice_collection_proxy::AddRef(void)
{
	return collection_->AddRef();
}

ULONG __stdcall mmdevice_collection_proxy::Release(void)
{
	auto count = collection_->Release();

	if (!count)
	{
		delete this;
	}

	return count;
}

HRESULT __stdcall mmdevice_collection_proxy::GetCount(UINT* pcDevices)
{
	return collection_->GetCount(pcDevices);
}
HRESULT __stdcall mmdevice_collection_proxy::Item(UINT nDevice, IMMDevice** ppDevice)
{
	auto hr = collection_->Item(nDevice, ppDevice);
	if (SUCCEEDED(hr))
	{
		*ppDevice = new mmdevice_proxy(*ppDevice);
	}
	return hr;
}