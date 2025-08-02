#pragma once

class mmdevice_collection_proxy : public IMMDeviceCollection
{
public:
	mmdevice_collection_proxy(IMMDeviceCollection* orig);

private:
	IMMDeviceCollection* collection_;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall GetCount(UINT* pcDevices) override;
	virtual HRESULT __stdcall Item(UINT nDevice, IMMDevice** ppDevice) override;
};
