#pragma once

#include "stdafx.h"
#include "d3d11.h"

#include "../LibOVR0.4/Include/OVR_CAPI_0_4_0.h"

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

EXTERNC HRESULT wrapCreateShaderResourceView(ID3D11Device* device, ID3D11Resource* resource, ID3D11ShaderResourceView** srv);
EXTERNC void initChains();
EXTERNC void setMirror(revMirrorTexture* mirror);
EXTERNC revMirrorTexture* getMirror();
EXTERNC void GetContext(ID3D11Device* device, ID3D11DeviceContext** context);
EXTERNC void CopyTexture(ID3D11DeviceContext* device, ID3D11Texture2D* dest, ovrTexture* src);

ovrBool ConfigureD3D11(revSession session, const ovrRenderAPIConfig* apiConfig, unsigned int distortionCaps,
	const ovrFovPort eyeFovIn[2], ovrEyeRenderDesc eyeRenderDescOut[2]);

void PresentD3D11(revSession session, const ovrPosef renderPose[2], const ovrTexture eyeTexture[2]);
void ShutdownD3D11();

#undef EXTERNC