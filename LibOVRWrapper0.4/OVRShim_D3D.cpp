#include "stdafx.h"

#undef OVR_PUBLIC_FUNCTION
#undef OVR_PUBLIC_CLASS
#undef OVR_PRIVATE_FUNCTION
#undef OVR_PRIVATE_CLASS

#if !defined(OVR_DLL_BUILD)
#define OVR_DLL_BUILD
#endif

#include "../LibOVR0.4/Include/OVR_CAPI_0_4_0.h"
#include "../LibOVR0.4/Include/OVR_CAPI_D3D.h"

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

//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D11 rendering.
struct OculusTexture
{
	revSession            Session;
	revTextureSwapChain   TextureChain;
	ovrSizei                  size;	
	std::vector<ID3D11Texture2D*> Tex2D;

	OculusTexture() :
		Session(nullptr),
		TextureChain(nullptr)
	{
	}

	bool Init(revSession session, ID3D11Device *device, ovrD3D11Texture texture)
	{
		Session = session;
		size.w = texture.D3D11.Header.TextureSize.w;
		size.h = texture.D3D11.Header.TextureSize.h;

		D3D11_TEXTURE2D_DESC orgdesc;
		texture.D3D11.pTexture->GetDesc(&orgdesc);

		revTextureSwapChainDesc desc = {};
		desc.Type = revTexture_2D;
		desc.ArraySize = 1;
		desc.Format = getOVRFormat(orgdesc.Format);
		desc.Width = texture.D3D11.Header.TextureSize.w;
		desc.Height = texture.D3D11.Header.TextureSize.h;
		desc.MipLevels = 1;
		desc.SampleCount = 1;
		desc.MiscFlags = 0;		
		desc.StaticImage = ovrFalse;

		switch (desc.Format) {
		case REV_FORMAT_R8G8B8A8_UNORM_SRGB:
		case REV_FORMAT_B8G8R8A8_UNORM_SRGB:
		case REV_FORMAT_B8G8R8X8_UNORM_SRGB:
			desc.MiscFlags |= revTextureMisc_DX_Typeless;
			break;
		}

		revResult result = rev_CreateTextureSwapChainDX(session, device, &desc, &TextureChain);
		if (!REV_SUCCESS(result))
			return false;

		int textureCount = 0;
		rev_GetTextureSwapChainLength(Session, TextureChain, &textureCount);
		for (int i = 0; i < textureCount; ++i)
		{
			ID3D11Texture2D* tex = nullptr;
			revResult r = rev_GetTextureSwapChainBufferDX(Session, TextureChain, i, IID_PPV_ARGS(&tex));
			if (r < 0) {
				BOOST_LOG_TRIVIAL(error) << "Init rev_GetTextureSwapChainBufferDX error " << r;
			}
			Tex2D.push_back(tex);
		}

		return true;
	}

	~OculusTexture()
	{		
		for (int i = 0;i < Tex2D.size();i++) {
			Tex2D[i]->Release();
		}
		if (TextureChain)
		{
			rev_DestroyTextureSwapChain(Session, TextureChain);
		}
	}

	ID3D11Texture2D* GetTex()
	{
		int index = 0;		
		revResult r = rev_GetTextureSwapChainCurrentIndex(Session, TextureChain, &index);
		if (r < 0) {
			BOOST_LOG_TRIVIAL(error) << "GetTex rev_GetTextureSwapChainCurrentIndex error " << r;
		}
		return this->Tex2D[index];
	}

	// Commit changes
	void Commit()
	{
		revResult r = rev_CommitTextureSwapChain(Session, TextureChain);
		if (r < 0) {
			BOOST_LOG_TRIVIAL(error) << "Commit rev_CommitTextureSwapChain error " << r;
		}
	}
};

ovrD3D11Config cfg;
ovrEyeRenderDesc EyeRenderDesc[2];
unsigned int globalDistortionCaps;
ovrFovPort eyeRenderFov[2];
OculusTexture  * pEyeRenderTexture[2] = { nullptr, nullptr };

// This gives us the D3D device, the mirror window's backbuffer render target, ovrDistortionCap_FlipInput, ovrDistortionCap_SRGB, ovrDistortionCap_HqDistortion,
// and the desired FOV. But it doesn't officially give us the render texture size that we need for creating the textures.
ovrBool ConfigureD3D11(revSession session, const ovrRenderAPIConfig* apiConfig, unsigned int distortionCaps,
	const ovrFovPort eyeFovIn[2], ovrEyeRenderDesc eyeRenderDescOut[2])
{
	cfg = *(ovrD3D11Config *)apiConfig;
	globalDistortionCaps = distortionCaps;
	for (int eye = 0; eye < 2; eye++) {
		eyeRenderFov[eye] = eyeFovIn[eye];
		EyeRenderDesc[eye] = eyeRenderDescOut[eye];
	}
	return ovrTrue;
}

extern unsigned int globalFrameIndex;
extern double globalTrackingStateTime;

void RecreateEyeRenderTexture(revSession session, int eye, ovrD3D11Texture texture) {
	bool dirty = false;
	if (!pEyeRenderTexture[eye]) {
		dirty = true;
	} else if (pEyeRenderTexture[eye]->size.w != texture.D3D11.Header.TextureSize.w || pEyeRenderTexture[eye]->size.h != texture.D3D11.Header.TextureSize.h) {
		delete pEyeRenderTexture[eye];
		pEyeRenderTexture[eye] = NULL;
		dirty = true;
	}
	if (dirty) {
		pEyeRenderTexture[eye] = new OculusTexture();
		if (!pEyeRenderTexture[eye]->Init(session, cfg.D3D11.pDevice, texture))
		{
			// failed
			BOOST_LOG_TRIVIAL(error) << "RecreateEyeRenderTexture Init error ";
		}
	}
}

void PresentD3D11(revSession session, const ovrPosef renderPose[2], const ovrTexture eyeTexture[2])
{
	ovrD3D11Texture* tex;
	tex = (ovrD3D11Texture*)eyeTexture;
	for (int eye = 0; eye < 2; ++eye)
	{
		RecreateEyeRenderTexture(session, eye, tex[eye]);
		//CopyEyeRenderTexture
		ID3D11Texture2D *dest = pEyeRenderTexture[eye]->GetTex();
		cfg.D3D11.pDeviceContext->CopyResource(dest, tex[eye].D3D11.pTexture);		
		pEyeRenderTexture[eye]->Commit();
	}

	// Initialize our single full screen Fov layer.
	revLayerHeader* layers[1];
	revLayerEyeFov ld = {};
	ld.Header.Type = revLayerType_EyeFov;
	ld.Header.Flags = 0;

	for (int eye = 0; eye < 2; ++eye)
	{
		ld.ColorTexture[eye] = pEyeRenderTexture[eye]->TextureChain;
		ld.Viewport[eye] = *(revRecti*)&tex[eye].D3D11.Header.RenderViewport;
		ld.Fov[eye].DownTan = eyeRenderFov[eye].DownTan;
		ld.Fov[eye].LeftTan = eyeRenderFov[eye].LeftTan;
		ld.Fov[eye].RightTan = eyeRenderFov[eye].RightTan;
		ld.Fov[eye].UpTan = eyeRenderFov[eye].UpTan;
		ld.RenderPose[eye].Orientation = *(revQuatf*)&renderPose[eye].Orientation;
		ld.RenderPose[eye].Position = *(revVector3f*)&renderPose[eye].Position;
		ld.SensorSampleTime = globalTrackingStateTime;
	}

	layers[0] = &ld.Header;
	rev_SubmitFrame(session, globalFrameIndex, NULL, layers, 1);
}

void ShutdownD3D11() {

	for (int i = 0;i < 2;i++) {
		if (pEyeRenderTexture[i] != nullptr) {
			delete pEyeRenderTexture[i];
			pEyeRenderTexture[i] = nullptr;
		}
	}
}