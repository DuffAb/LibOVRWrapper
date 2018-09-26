/********************************************************************************//**
\file      REV_CAPI_D3D.h
\brief     D3D specific structures used by the CAPI interface.
\copyright Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
************************************************************************************/

#ifndef REV_CAPI_D3D__h
#define REV_CAPI_D3D__h

#include "REV_CAPI.h"
#include "REV_Version.h"


#if defined (_WIN32)
#include <Unknwn.h>

//-----------------------------------------------------------------------------------
// ***** Direct3D Specific

/// Create Texture Swap Chain suitable for use with Direct3D 11 and 12.
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  d3dPtr Specifies the application's D3D11Device to create resources with or the D3D12CommandQueue
///             which must be the same one the application renders to the eye textures with.
/// \param[in]  desc Specifies requested texture properties. See notes for more info about texture format.
/// \param[in]  bindFlags Specifies what revTextureBindFlags the application requires for this texture chain.
/// \param[out] out_TextureSwapChain Returns the created revTextureSwapChain, which will be valid upon a successful return value, else it will be NULL.
///             This texture chain must be eventually destroyed via rev_DestroyTextureSwapChain before destroying the HMD with rev_Destroy.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
/// \note The texture format provided in \a desc should be thought of as the format the distortion-compositor will use for the
/// ShaderResourceView when reading the contents of the texture. To that end, it is highly recommended that the application
/// requests texture swapchain formats that are in sRGB-space (e.g. REV_FORMAT_R8G8B8A8_UNORM_SRGB) as the compositor
/// does sRGB-correct rendering. As such, the compositor relies on the GPU's hardware sampler to do the sRGB-to-linear
/// conversion. If the application still prefers to render to a linear format (e.g. REV_FORMAT_R8G8B8A8_UNORM) while handling the
/// linear-to-gamma conversion via HLSL code, then the application must still request the corresponding sRGB format and also use
/// the \a revTextureMisc_DX_Typeless flag in the revTextureSwapChainDesc's Flag field. This will allow the application to create
/// a RenderTargetView that is the desired linear format while the compositor continues to treat it as sRGB. Failure to do so
/// will cause the compositor to apply unexpected gamma conversions leading to gamma-curve artifacts. The \a revTextureMisc_DX_Typeless
/// flag for depth buffer formats (e.g. REV_FORMAT_D32_FLOAT) is ignored as they are always converted to be typeless.
///
/// \see rev_GetTextureSwapChainLength
/// \see rev_GetTextureSwapChainCurrentIndex
/// \see rev_GetTextureSwapChainDesc
/// \see rev_GetTextureSwapChainBufferDX
/// \see rev_DestroyTextureSwapChain
///
REV_PUBLIC_FUNCTION(revResult) rev_CreateTextureSwapChainDX(revSession session,
                                                            IUnknown* d3dPtr,
                                                            const revTextureSwapChainDesc* desc,
                                                            revTextureSwapChain* out_TextureSwapChain);


/// Get a specific buffer within the chain as any compatible COM interface (similar to QueryInterface)
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  chain Specifies an revTextureSwapChain previously returned by rev_CreateTextureSwapChainDX
/// \param[in]  index Specifies the index within the chain to retrieve. Must be between 0 and length (see rev_GetTextureSwapChainLength),
///             or may pass -1 to get the buffer at the CurrentIndex location. (Saving a call to GetTextureSwapChainCurrentIndex)
/// \param[in]  iid Specifies the interface ID of the interface pointer to query the buffer for.
/// \param[out] out_Buffer Returns the COM interface pointer retrieved.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
/// <b>Example code</b>
///     \code{.cpp}
///         rev_GetTextureSwapChainBuffer(session, chain, 0, IID_ID3D11Texture2D, &d3d11Texture);
///         rev_GetTextureSwapChainBuffer(session, chain, 1, IID_PPV_ARGS(&dxgiResource));
///     \endcode
///
REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainBufferDX(revSession session,
                                                               revTextureSwapChain chain,
                                                               int index,
                                                               IID iid,
                                                               void** out_Buffer);


/// Create Mirror Texture which is auto-refreshed to mirror Rift contents produced by this application.
///
/// A second call to rev_CreateMirrorTextureDX for a given revSession before destroying the first one
/// is not supported and will result in an error return.
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  d3dPtr Specifies the application's D3D11Device to create resources with or the D3D12CommandQueue
///             which must be the same one the application renders to the textures with.
/// \param[in]  desc Specifies requested texture properties. See notes for more info about texture format.
/// \param[out] out_MirrorTexture Returns the created revMirrorTexture, which will be valid upon a successful return value, else it will be NULL.
///             This texture must be eventually destroyed via rev_DestroyMirrorTexture before destroying the HMD with rev_Destroy.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
/// \note The texture format provided in \a desc should be thought of as the format the compositor will use for the RenderTargetView when
/// writing into mirror texture. To that end, it is highly recommended that the application requests a mirror texture format that is
/// in sRGB-space (e.g. REV_FORMAT_R8G8B8A8_UNORM_SRGB) as the compositor does sRGB-correct rendering. If however the application wants
/// to still read the mirror texture as a linear format (e.g. REV_FORMAT_R8G8B8A8_UNORM) and handle the sRGB-to-linear conversion in
/// HLSL code, then it is recommended the application still requests an sRGB format and also use the \a revTextureMisc_DX_Typeless flag in the
/// revMirrorTextureDesc's Flags field. This will allow the application to bind a ShaderResourceView that is a linear format while the
/// compositor continues to treat is as sRGB. Failure to do so will cause the compositor to apply unexpected gamma conversions leading to 
/// gamma-curve artifacts.
///
/// \see rev_GetMirrorTextureBufferDX
/// \see rev_DestroyMirrorTexture
///
REV_PUBLIC_FUNCTION(revResult) rev_CreateMirrorTextureDX(revSession session,
                                                         IUnknown* d3dPtr,
                                                         const revMirrorTextureDesc* desc,
                                                         revMirrorTexture* out_MirrorTexture);

/// Get a the underlying buffer as any compatible COM interface (similar to QueryInterface) 
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  mirrorTexture Specifies an revMirrorTexture previously returned by rev_CreateMirrorTextureDX
/// \param[in]  iid Specifies the interface ID of the interface pointer to query the buffer for.
/// \param[out] out_Buffer Returns the COM interface pointer retrieved.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetMirrorTextureBufferDX(revSession session,
                                                            revMirrorTexture mirrorTexture,
                                                            IID iid,
                                                            void** out_Buffer);


#endif // _WIN32

#endif    // REV_CAPI_D3D_h
