#include "stdafx.h"

#undef OVR_PUBLIC_FUNCTION
#undef OVR_PUBLIC_CLASS
#undef OVR_PRIVATE_FUNCTION
#undef OVR_PRIVATE_CLASS

#if !defined(OVR_DLL_BUILD)
#define OVR_DLL_BUILD
#endif

#include "../LibOVR0.6/Include/OVR_CAPI_0_6_0.h"
#include "../LibOVR0.6/Include/OVR_CAPI_D3D.h"

#include "shimhelper.h"

revTextureFormat getOVRFormat(DXGI_FORMAT format) {
	switch (format) {
		case DXGI_FORMAT_B5G6R5_UNORM:
			return REV_FORMAT_B5G6R5_UNORM;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return REV_FORMAT_B5G5R5A1_UNORM;
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return REV_FORMAT_B4G4R4A4_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return REV_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return REV_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return REV_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return REV_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return REV_FORMAT_B8G8R8X8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return REV_FORMAT_B8G8R8X8_UNORM_SRGB;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return REV_FORMAT_R16G16B16A16_FLOAT;
		case DXGI_FORMAT_D16_UNORM:
			return REV_FORMAT_D16_UNORM;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return REV_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_D32_FLOAT:
			return REV_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			return REV_FORMAT_D32_FLOAT_S8X24_UINT;
		case DXGI_FORMAT_R32_TYPELESS:
			return REV_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_R24G8_TYPELESS:
			return REV_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R16_TYPELESS:
			return REV_FORMAT_D16_UNORM;
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return REV_FORMAT_D32_FLOAT_S8X24_UINT;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return REV_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return REV_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			return REV_FORMAT_B8G8R8X8_UNORM_SRGB;
	}
	
	return REV_FORMAT_UNKNOWN;		
}

inline DXGI_FORMAT getShaderResourceFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:    return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:    return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:    return DXGI_FORMAT_B8G8R8X8_UNORM;
	case DXGI_FORMAT_BC1_TYPELESS:         return DXGI_FORMAT_BC1_UNORM;
	case DXGI_FORMAT_BC2_TYPELESS:         return DXGI_FORMAT_BC2_UNORM;
	case DXGI_FORMAT_BC3_TYPELESS:         return DXGI_FORMAT_BC3_UNORM;
	case DXGI_FORMAT_BC7_TYPELESS:         return DXGI_FORMAT_BC7_UNORM;	
	case DXGI_FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R16_TYPELESS: return DXGI_FORMAT_R16_UNORM;
	}
	return format;
}


OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_CreateSwapTextureSetD3D11(ovrHmd hmd,
	ID3D11Device* device,
	const D3D11_TEXTURE2D_DESC* desc,
	ovrSwapTextureSet** outTextureSet) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_CreateSwapTextureSetD3D11 format " << desc->Format << " samples " << desc->SampleDesc.Count << " bindflags " << desc->BindFlags << " miscflags " << desc->MiscFlags;

	D3D11_TEXTURE2D_DESC descClone;
	memcpy(&descClone, desc, sizeof(D3D11_TEXTURE2D_DESC));

	revTextureSwapChainDesc d;
	d.Type = revTexture_2D;
	d.ArraySize = desc->ArraySize;
	
	d.Format = getOVRFormat(desc->Format);

	if (d.Format == 0) {
		BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateSwapTextureSetD3D11 unknown format";
		return -1;
	}
	
	d.Width = desc->Width;
	d.Height = desc->Height;
	d.MipLevels = desc->MipLevels;
	d.SampleCount = desc->SampleDesc.Count;
	
	d.MiscFlags = 0;
	
	switch (d.Format) {
	case REV_FORMAT_R8G8B8A8_UNORM_SRGB:	
	case REV_FORMAT_B8G8R8A8_UNORM_SRGB:	
	case REV_FORMAT_B8G8R8X8_UNORM_SRGB:
		d.MiscFlags |= revTextureMisc_DX_Typeless;		
		break;
	}

	if (getWrapperSettings()->srgbCorrectionEnabled) {
		switch (d.Format) {
		case REV_FORMAT_R8G8B8A8_UNORM_SRGB:
			descClone.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case REV_FORMAT_B8G8R8A8_UNORM_SRGB:
			descClone.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		case REV_FORMAT_B8G8R8X8_UNORM_SRGB:
			descClone.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
			break;
		}
	}	
	
	bool makeShaderView = false;
	d.BindFlags = 0;
	/*if (desc->BindFlags & D3D11_BIND_RENDER_TARGET) {
		d.BindFlags |= ovrTextureBind_DX_RenderTarget;
	}
	if (desc->BindFlags & D3D11_BIND_UNORDERED_ACCESS) {
		d.BindFlags |= ovrTextureBind_DX_UnorderedAccess;
	}*/
	if (desc->BindFlags & D3D11_BIND_DEPTH_STENCIL) {
		d.MiscFlags |= revTextureMisc_DX_Typeless;
		d.BindFlags |= revTextureBind_DX_DepthStencil;
	}	
	if (desc->BindFlags & D3D11_BIND_SHADER_RESOURCE) {
		
		makeShaderView = true;
	}
	
	d.StaticImage = ovrFalse;

	ovrTextureSwapChainWrapper* chainwrapper = (ovrTextureSwapChainWrapper*)malloc(sizeof(ovrTextureSwapChainWrapper));
	device->GetImmediateContext(&chainwrapper->pContext);

	ovrResult result = rev_CreateTextureSwapChainDX((revSession)hmd->Handle, (IUnknown*)device, &d, &chainwrapper->swapChain);

	if (!OVR_SUCCESS(result)) {
		BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateSwapTextureSetD3D11 could not create TextureSwapChain";
		return result;
	}
	
	ovrSwapTextureSet* ts = (ovrSwapTextureSet*)malloc(sizeof(ovrSwapTextureSet));

	setChain((revSession)hmd->Handle, ts, chainwrapper);

	rev_GetTextureSwapChainLength((revSession)hmd->Handle, chainwrapper->swapChain, &chainwrapper->textureCount);

	chainwrapper->textures = (ID3D11Texture2D**)calloc(chainwrapper->textureCount, sizeof(ID3D11Texture2D*));

	ts->TextureCount = 2;
	ts->CurrentIndex = 0;	
	ts->Textures = (ovrTexture*)calloc(ts->TextureCount, sizeof(ovrD3D11Texture));

	for (int i = 0; i < chainwrapper->textureCount; ++i)
	{		
		result = rev_GetTextureSwapChainBufferDX((revSession)hmd->Handle, chainwrapper->swapChain, i, IID_ID3D11Texture2D, (void**)&chainwrapper->textures[i]);
		if (!OVR_SUCCESS(result)) {
			BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateSwapTextureSetD3D11 could not allocate TextureSwapChainBuffer";
			return result;
		}
	}

	for (int i = 0;i < 2;i++) {
		ovrD3D11Texture* ovrtext = (ovrD3D11Texture*)&ts->Textures[i];

		HRESULT hr = device->CreateTexture2D(&descClone, nullptr, &ovrtext->D3D11.pTexture);

		if (hr < 0) {
			BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateSwapTextureSetD3D11 could create texture";
			return ovrError_ServiceError;
		}

		if (makeShaderView) {
			HRESULT rs;

			D3D11_SHADER_RESOURCE_VIEW_DESC depthSrv;
			ZeroMemory(&depthSrv, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
			depthSrv.Format = getShaderResourceFormat(descClone.Format);
			depthSrv.ViewDimension = desc->SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			depthSrv.Texture2D.MostDetailedMip = 0;
			depthSrv.Texture2D.MipLevels = desc->MipLevels;
			
			rs = device->CreateShaderResourceView((ID3D11Resource*)ovrtext->D3D11.pTexture, &depthSrv, &(ovrtext->D3D11.pSRView));
			
			if (rs < 0) {
				BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateSwapTextureSetD3D11 could not create ShaderResourceView";
				return ovrError_ServiceError;
			}
		}

		ovrtext->D3D11.Header.API = ovrRenderAPI_D3D11;
		ovrtext->D3D11.Header.TextureSize.w = d.Width;
		ovrtext->D3D11.Header.TextureSize.h = d.Height;
	}
	
	*outTextureSet = ts;	

	return result;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_CreateMirrorTextureD3D11(ovrHmd hmd,
	ID3D11Device* device,
	const D3D11_TEXTURE2D_DESC* desc,
	ovrTexture** outMirrorTexture) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_CreateMirrorTextureD3D11 format " << desc->Format << " samples " << desc->SampleDesc.Count << " bindflags " << desc->BindFlags << " miscflags " << desc->MiscFlags;

	revMirrorTextureDesc d;
	
	d.Format = getOVRFormat(desc->Format);

	d.Width = desc->Width;
	d.Height = desc->Height;

	d.MiscFlags = 0;

	switch (d.Format) {
	case REV_FORMAT_R8G8B8A8_UNORM_SRGB:
	case REV_FORMAT_B8G8R8A8_UNORM_SRGB:
	case REV_FORMAT_B8G8R8X8_UNORM_SRGB:
		d.MiscFlags |= revTextureMisc_DX_Typeless;		
		break;
	}

	revMirrorTexture* mirror = (revMirrorTexture*)malloc(sizeof(revMirrorTexture));

	ovrResult result = rev_CreateMirrorTextureDX((revSession)hmd->Handle, (IUnknown*)device, &d, mirror);

	if (!REV_SUCCESS(result)) {
		revErrorInfo info;
		rev_GetLastErrorInfo(&info);

		BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateMirrorTextureD3D11 could not allocate Mirrortexture:" << info.ErrorString;
		return result;
	}

	ovrD3D11Texture* ovrtext = (ovrD3D11Texture*)malloc(sizeof(ovrD3D11Texture));

	ID3D11Texture2D* texture = 0;	
	rev_GetMirrorTextureBufferDX((revSession)hmd->Handle, *mirror, IID_ID3D11Texture2D, (void**)&texture);

	ovrtext->D3D11.pTexture = texture;

	if (desc->BindFlags & D3D11_BIND_SHADER_RESOURCE) {
		HRESULT rs;

		D3D11_SHADER_RESOURCE_VIEW_DESC depthSrv;
		ZeroMemory(&depthSrv, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		depthSrv.Format = getShaderResourceFormat(desc->Format);
		depthSrv.ViewDimension = desc->SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		depthSrv.Texture2D.MostDetailedMip = 0;
		depthSrv.Texture2D.MipLevels = desc->MipLevels;

		rs = device->CreateShaderResourceView((ID3D11Resource*)ovrtext->D3D11.pTexture, &depthSrv, &(ovrtext->D3D11.pSRView));

		if (rs < 0) {
			BOOST_LOG_TRIVIAL(error) << "ovrHmd_CreateMirrorTextureD3D11 could not create ShaderResourceView";
			return ovrError_ServiceError;
		}
	}

	ovrtext->D3D11.Header.API = ovrRenderAPI_D3D11;
	ovrtext->D3D11.Header.TextureSize.w = d.Width;
	ovrtext->D3D11.Header.TextureSize.h = d.Height;

	*outMirrorTexture = (ovrTexture*)ovrtext;

	setMirror(mirror);

	return result;

}