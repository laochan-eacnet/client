#pragma once

class mmdevice_proxy : public IMMDevice
{
public:
	mmdevice_proxy(IMMDevice* orig);

private:
	IMMDevice* device_;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG __stdcall AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	virtual HRESULT __stdcall Activate(REFIID iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface) override;
	virtual HRESULT __stdcall OpenPropertyStore(DWORD stgmAccess, IPropertyStore** ppProperties) override;
	virtual HRESULT __stdcall GetId(LPWSTR* ppstrId) override;
	virtual HRESULT __stdcall GetState(DWORD* pdwState) override;
};