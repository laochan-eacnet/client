#include <std_include.hpp>
#include "interface_ex.hpp"

d3d9ex_device_proxy::d3d9ex_device_proxy(IDirect3DDevice9Ex* orig, int width, int height) :
	device_(orig), width_(width), height_(height)
{
}

HRESULT __stdcall d3d9ex_device_proxy::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = nullptr;

	auto hr = device_->QueryInterface(riid, ppvObj);
	if (hr == NOERROR) *ppvObj = this;

	return hr;
}

ULONG __stdcall d3d9ex_device_proxy::AddRef(void)
{
	return device_->AddRef();
}

ULONG __stdcall d3d9ex_device_proxy::Release(void)
{
	auto count = device_->Release();

	if (!count)
	{
		delete this;
	}

	return count;
}

HRESULT __stdcall d3d9ex_device_proxy::TestCooperativeLevel(void)
{
	return device_->TestCooperativeLevel();
}

UINT __stdcall d3d9ex_device_proxy::GetAvailableTextureMem(void)
{
	return device_->GetAvailableTextureMem();
}

HRESULT __stdcall d3d9ex_device_proxy::EvictManagedResources(void)
{
	return device_->EvictManagedResources();
}

HRESULT __stdcall d3d9ex_device_proxy::GetDirect3D(IDirect3D9** ppD3D9)
{
	return device_->GetDirect3D(ppD3D9);
}

HRESULT __stdcall d3d9ex_device_proxy::GetDeviceCaps(D3DCAPS9* pCaps)
{
	return device_->GetDeviceCaps(pCaps);
}

HRESULT __stdcall d3d9ex_device_proxy::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return device_->GetDisplayMode(iSwapChain, pMode);
}

HRESULT __stdcall d3d9ex_device_proxy::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
	return device_->GetCreationParameters(pParameters);
}

HRESULT __stdcall d3d9ex_device_proxy::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	return device_->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void __stdcall d3d9ex_device_proxy::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return device_->SetCursorPosition(X, Y, Flags);
}

BOOL __stdcall d3d9ex_device_proxy::ShowCursor(BOOL bShow)
{
	return device_->ShowCursor(true);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	return device_->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT __stdcall d3d9ex_device_proxy::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	return device_->GetSwapChain(iSwapChain, pSwapChain);
}

UINT __stdcall d3d9ex_device_proxy::GetNumberOfSwapChains(void)
{
	return device_->GetNumberOfSwapChains();
}

HRESULT __stdcall d3d9ex_device_proxy::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	return device_->Reset(pPresentationParameters);
}

HRESULT __stdcall d3d9ex_device_proxy::Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	return device_->PresentEx(pSourceRect, pDestRect, NULL, NULL, D3DPRESENT_FORCEIMMEDIATE);
}

HRESULT __stdcall d3d9ex_device_proxy::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	return device_->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT __stdcall d3d9ex_device_proxy::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return device_->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT __stdcall d3d9ex_device_proxy::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return device_->SetDialogBoxMode(bEnableDialogs);
}

void __stdcall d3d9ex_device_proxy::SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP* pRamp)
{
	return device_->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void __stdcall d3d9ex_device_proxy::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	return device_->GetGammaRamp(iSwapChain, pRamp);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	return device_->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return device_->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return device_->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return device_->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return device_->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	printf("create render target: %u %u %x\n", Width, Height, Format);

	if (width_ > 1920 && height_ > 1080 && Width == 1920 && Height == 1080)
	{
		Width = width_;
		Height = height_;

		printf("overridding render size: %u %u\n", Width, Height);
	}

	return device_->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return device_->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::UpdateSurface(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, const POINT* pDestPoint)
{
	return device_->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT __stdcall d3d9ex_device_proxy::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	return device_->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT __stdcall d3d9ex_device_proxy::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return device_->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT __stdcall d3d9ex_device_proxy::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return device_->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT __stdcall d3d9ex_device_proxy::StretchRect(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestSurface, const RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return device_->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT __stdcall d3d9ex_device_proxy::ColorFill(IDirect3DSurface9* pSurface, const RECT* pRect, D3DCOLOR color)
{
	return device_->ColorFill(pSurface, pRect, color);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return device_->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT __stdcall d3d9ex_device_proxy::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	D3DSURFACE_DESC desc;
	pRenderTarget->GetDesc(&desc);

	return device_->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT __stdcall d3d9ex_device_proxy::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return device_->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT __stdcall d3d9ex_device_proxy::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	return device_->SetDepthStencilSurface(pNewZStencil);
}

HRESULT __stdcall d3d9ex_device_proxy::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	return device_->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT __stdcall d3d9ex_device_proxy::BeginScene(void)
{
	return device_->BeginScene();
}

HRESULT __stdcall d3d9ex_device_proxy::EndScene(void)
{
	return device_->EndScene();
}

HRESULT __stdcall d3d9ex_device_proxy::Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return device_->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT __stdcall d3d9ex_device_proxy::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix)
{
	return device_->SetTransform(State, pMatrix);
}

HRESULT __stdcall d3d9ex_device_proxy::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	return device_->GetTransform(State, pMatrix);
}

HRESULT __stdcall d3d9ex_device_proxy::MultiplyTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* Matrix)
{
	return device_->MultiplyTransform(Type, Matrix);
}

HRESULT __stdcall d3d9ex_device_proxy::SetViewport(const D3DVIEWPORT9* pViewport)
{
	auto vp = const_cast<D3DVIEWPORT9*>(pViewport);
	if (width_ > 1920 && height_ > 1080 && vp->Width == 1920 && vp->Height == 1080)
	{
		vp->Width = width_;
		vp->Height = height_;
	}

	return device_->SetViewport(pViewport);
}

HRESULT __stdcall d3d9ex_device_proxy::GetViewport(D3DVIEWPORT9* pViewport)
{
	return device_->GetViewport(pViewport);
}

HRESULT __stdcall d3d9ex_device_proxy::SetMaterial(const D3DMATERIAL9* pMaterial)
{
	return device_->SetMaterial(pMaterial);
}

HRESULT __stdcall d3d9ex_device_proxy::GetMaterial(D3DMATERIAL9* pMaterial)
{
	return device_->GetMaterial(pMaterial);
}

HRESULT __stdcall d3d9ex_device_proxy::SetLight(DWORD Index, const D3DLIGHT9* Light)
{
	return device_->SetLight(Index, Light);
}

HRESULT __stdcall d3d9ex_device_proxy::GetLight(DWORD Index, D3DLIGHT9* Light)
{
	return device_->GetLight(Index, Light);
}

HRESULT __stdcall d3d9ex_device_proxy::LightEnable(DWORD Index, BOOL Enable)
{
	return device_->LightEnable(Index, Enable);
}

HRESULT __stdcall d3d9ex_device_proxy::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	return device_->GetLightEnable(Index, pEnable);
}

HRESULT __stdcall d3d9ex_device_proxy::SetClipPlane(DWORD Index, const float* pPlane)
{
	return device_->SetClipPlane(Index, pPlane);
}

HRESULT __stdcall d3d9ex_device_proxy::GetClipPlane(DWORD Index, float* pPlane)
{
	return device_->GetClipPlane(Index, pPlane);
}

HRESULT __stdcall d3d9ex_device_proxy::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return device_->SetRenderState(State, Value);
}

HRESULT __stdcall d3d9ex_device_proxy::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	return device_->GetRenderState(State, pValue);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return device_->CreateStateBlock(Type, ppSB);
}

HRESULT __stdcall d3d9ex_device_proxy::BeginStateBlock(void)
{
	return device_->BeginStateBlock();
}

HRESULT __stdcall d3d9ex_device_proxy::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	return device_->EndStateBlock(ppSB);
}

HRESULT __stdcall d3d9ex_device_proxy::SetClipStatus(const D3DCLIPSTATUS9* pClipStatus)
{
	return device_->SetClipStatus(pClipStatus);
}

HRESULT __stdcall d3d9ex_device_proxy::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	return device_->GetClipStatus(pClipStatus);
}

HRESULT __stdcall d3d9ex_device_proxy::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	return device_->GetTexture(Stage, ppTexture);
}

HRESULT __stdcall d3d9ex_device_proxy::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	return device_->SetTexture(Stage, pTexture);
}

HRESULT __stdcall d3d9ex_device_proxy::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	return device_->GetTextureStageState(Stage, Type, pValue);
}

HRESULT __stdcall d3d9ex_device_proxy::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return device_->SetTextureStageState(Stage, Type, Value);
}

HRESULT __stdcall d3d9ex_device_proxy::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return device_->GetSamplerState(Sampler, Type, pValue);
}

HRESULT __stdcall d3d9ex_device_proxy::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return device_->SetSamplerState(Sampler, Type, Value);
}

HRESULT __stdcall d3d9ex_device_proxy::ValidateDevice(DWORD* pNumPasses)
{
	return device_->ValidateDevice(pNumPasses);
}

HRESULT __stdcall d3d9ex_device_proxy::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY* pEntries)
{
	return device_->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT __stdcall d3d9ex_device_proxy::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	return device_->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT __stdcall d3d9ex_device_proxy::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return device_->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT __stdcall d3d9ex_device_proxy::GetCurrentTexturePalette(UINT* PaletteNumber)
{
	return device_->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT __stdcall d3d9ex_device_proxy::SetScissorRect(const RECT* pRect)
{
	return device_->SetScissorRect(pRect);
}

HRESULT __stdcall d3d9ex_device_proxy::GetScissorRect(RECT* pRect)
{
	return device_->GetScissorRect(pRect);
}

HRESULT __stdcall d3d9ex_device_proxy::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return device_->SetSoftwareVertexProcessing(bSoftware);
}

BOOL __stdcall d3d9ex_device_proxy::GetSoftwareVertexProcessing(void)
{
	return device_->GetSoftwareVertexProcessing();
}

HRESULT __stdcall d3d9ex_device_proxy::SetNPatchMode(float nSegments)
{
	return device_->SetNPatchMode(nSegments);
}

float __stdcall d3d9ex_device_proxy::GetNPatchMode(void)
{
	return device_->GetNPatchMode();
}

HRESULT __stdcall d3d9ex_device_proxy::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return device_->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT __stdcall d3d9ex_device_proxy::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return device_->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT __stdcall d3d9ex_device_proxy::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return device_->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT __stdcall d3d9ex_device_proxy::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return device_->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT __stdcall d3d9ex_device_proxy::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return device_->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateVertexDeclaration(const D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return device_->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT __stdcall d3d9ex_device_proxy::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return device_->SetVertexDeclaration(pDecl);
}

HRESULT __stdcall d3d9ex_device_proxy::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return device_->GetVertexDeclaration(ppDecl);
}

HRESULT __stdcall d3d9ex_device_proxy::SetFVF(DWORD FVF)
{
	return device_->SetFVF(FVF);
}

HRESULT __stdcall d3d9ex_device_proxy::GetFVF(DWORD* pFVF)
{
	return device_->GetFVF(pFVF);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateVertexShader(const DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	return device_->CreateVertexShader(pFunction, ppShader);
}

HRESULT __stdcall d3d9ex_device_proxy::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return device_->SetVertexShader(pShader);
}

HRESULT __stdcall d3d9ex_device_proxy::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return device_->GetVertexShader(ppShader);
}

HRESULT __stdcall d3d9ex_device_proxy::SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
	return device_->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT __stdcall d3d9ex_device_proxy::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return device_->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT __stdcall d3d9ex_device_proxy::SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount)
{
	return device_->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT __stdcall d3d9ex_device_proxy::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return device_->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT __stdcall d3d9ex_device_proxy::SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount)
{
	return device_->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT __stdcall d3d9ex_device_proxy::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return device_->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT __stdcall d3d9ex_device_proxy::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return device_->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT __stdcall d3d9ex_device_proxy::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride)
{
	return device_->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT __stdcall d3d9ex_device_proxy::SetStreamSourceFreq(UINT StreamNumber, UINT Setting)
{
	return device_->SetStreamSourceFreq(StreamNumber, Setting);
}

HRESULT __stdcall d3d9ex_device_proxy::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting)
{
	return device_->GetStreamSourceFreq(StreamNumber, pSetting);
}

HRESULT __stdcall d3d9ex_device_proxy::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return device_->SetIndices(pIndexData);
}

HRESULT __stdcall d3d9ex_device_proxy::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return device_->GetIndices(ppIndexData);
}

HRESULT __stdcall d3d9ex_device_proxy::CreatePixelShader(const DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	return device_->CreatePixelShader(pFunction, ppShader);
}

HRESULT __stdcall d3d9ex_device_proxy::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return device_->SetPixelShader(pShader);
}

HRESULT __stdcall d3d9ex_device_proxy::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return device_->GetPixelShader(ppShader);
}

HRESULT __stdcall d3d9ex_device_proxy::SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
	return device_->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT __stdcall d3d9ex_device_proxy::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return device_->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT __stdcall d3d9ex_device_proxy::SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount)
{
	return device_->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT __stdcall d3d9ex_device_proxy::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return device_->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT __stdcall d3d9ex_device_proxy::SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount)
{
	return device_->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT __stdcall d3d9ex_device_proxy::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return device_->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT __stdcall d3d9ex_device_proxy::DrawRectPatch(UINT Handle, const float* pNumSegs, const D3DRECTPATCH_INFO* pRectPatchInfo)
{
	return device_->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT __stdcall d3d9ex_device_proxy::DrawTriPatch(UINT Handle, const float* pNumSegs, const D3DTRIPATCH_INFO* pTriPatchInfo)
{
	return device_->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT __stdcall d3d9ex_device_proxy::DeletePatch(UINT Handle)
{
	return device_->DeletePatch(Handle);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return device_->CreateQuery(Type, ppQuery);
}

HRESULT __stdcall d3d9ex_device_proxy::SetConvolutionMonoKernel(UINT width, UINT height, float* rows, float* columns)
{
	return device_->SetConvolutionMonoKernel(width, height, rows, columns);
}

HRESULT __stdcall d3d9ex_device_proxy::ComposeRects(IDirect3DSurface9* pSrc, IDirect3DSurface9* pDst, IDirect3DVertexBuffer9* pSrcRectDescs, UINT NumRects, IDirect3DVertexBuffer9* pDstRectDescs, D3DCOMPOSERECTSOP Operation, int Xoffset, int Yoffset)
{
	return device_->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
}

HRESULT __stdcall d3d9ex_device_proxy::PresentEx(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	return device_->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

HRESULT __stdcall d3d9ex_device_proxy::GetGPUThreadPriority(INT* pPriority)
{
	return device_->GetGPUThreadPriority(pPriority);
}

HRESULT __stdcall d3d9ex_device_proxy::SetGPUThreadPriority(INT Priority)
{
	return device_->SetGPUThreadPriority(Priority);
}

HRESULT __stdcall d3d9ex_device_proxy::WaitForVBlank(UINT iSwapChain)
{
	return device_->WaitForVBlank(iSwapChain);
}

HRESULT __stdcall d3d9ex_device_proxy::CheckResourceResidency(IDirect3DResource9** pResourceArray, UINT32 NumResources)
{
	return device_->CheckResourceResidency(pResourceArray, NumResources);
}

HRESULT __stdcall d3d9ex_device_proxy::SetMaximumFrameLatency(UINT MaxLatency)
{
	return device_->SetMaximumFrameLatency(MaxLatency);
}

HRESULT __stdcall d3d9ex_device_proxy::GetMaximumFrameLatency(UINT* pMaxLatency)
{
	return device_->GetMaximumFrameLatency(pMaxLatency);
}

HRESULT __stdcall d3d9ex_device_proxy::CheckDeviceState(HWND hDestinationWindow)
{
	return device_->CheckDeviceState(hDestinationWindow);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateRenderTargetEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	printf("create rt ex %d, %d\n", Width, Height);

	return device_->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateOffscreenPlainSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	return device_->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
}

HRESULT __stdcall d3d9ex_device_proxy::CreateDepthStencilSurfaceEx(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage)
{
	return device_->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
}

HRESULT __stdcall d3d9ex_device_proxy::ResetEx(D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode)
{
	return device_->ResetEx(pPresentationParameters, pFullscreenDisplayMode);
}

HRESULT __stdcall d3d9ex_device_proxy::GetDisplayModeEx(UINT iSwapChain, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
{
	return device_->GetDisplayModeEx(iSwapChain, pMode, pRotation);
}

