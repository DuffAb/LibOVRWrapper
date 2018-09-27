#pragma once

#include "stdafx.h"
#include "d3d11.h"

#include "../LibOVR0.6/Include/OVR_CAPI_0_6_0.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef struct ovrTextureSwapChainWrapper_
{
	revTextureSwapChain swapChain;
	int textureCount;
	ID3D11Texture2D** textures;	
	ID3D11DeviceContext* pContext;	
} ovrTextureSwapChainWrapper;

WrapperSettings* getWrapperSettings();
void setWrapperSettings(WrapperSettings* settings);

EXTERNC HRESULT wrapCreateShaderResourceView(ID3D11Device* device, ID3D11Resource* resource, ID3D11ShaderResourceView** srv);
EXTERNC ovrTextureSwapChainWrapper* getChain(revSession session, ovrSwapTextureSet* ts);
EXTERNC void setChain(revSession session, ovrSwapTextureSet* ts, ovrTextureSwapChainWrapper* chain);
EXTERNC void initChains();
EXTERNC void removeChain(revSession session, ovrSwapTextureSet* ts);
EXTERNC void setMirror(revMirrorTexture* mirror);
EXTERNC revMirrorTexture* getMirror();
EXTERNC ovrResult makeD3D11Texture(IUnknown* device,
	const D3D11_TEXTURE2D_DESC* desc,
	ID3D11Texture2D** outTexture);
EXTERNC void GetContext(ID3D11Device* device, ID3D11DeviceContext** context);
EXTERNC void CopyTexture(ID3D11DeviceContext* device, ID3D11Texture2D* dest, ovrTexture* src);

#undef EXTERNC