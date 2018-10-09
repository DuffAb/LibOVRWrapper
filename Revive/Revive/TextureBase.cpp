#include "TextureBase.h"

ovrTextureSwapChainData::ovrTextureSwapChainData(vr::ETextureType api, ovrTextureSwapChainDesc desc)
	: ApiType(api)
	, Length(REV_SWAPCHAIN_LENGTH)
	, Identifier(0)
	, CurrentIndex(0)
	, Desc(desc)
	, Overlay(vr::k_ulOverlayHandleInvalid)
{
	memset(Textures, 0, sizeof(Textures));
}

ovrMirrorTextureData::ovrMirrorTextureData(vr::ETextureType api, ovrMirrorTextureDesc desc)
	: ApiType(api)
	, Desc(desc)
{
}
