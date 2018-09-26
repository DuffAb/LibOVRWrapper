/********************************************************************************//**
\file      REV_CAPI_GL.h
\brief     OpenGL-specific structures used by the CAPI interface.
\copyright Copyright 2015 Oculus VR, LLC. All Rights reserved.
************************************************************************************/

#ifndef REV_CAPI_GL__h
#define REV_CAPI_GL__h

#include "REV_CAPI.h"

/// Creates a TextureSwapChain suitable for use with OpenGL.
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  desc Specifies the requested texture properties. See notes for more info about texture format.
/// \param[out] out_TextureSwapChain Returns the created revTextureSwapChain, which will be valid upon
///             a successful return value, else it will be NULL. This texture swap chain must be eventually
///             destroyed via rev_DestroyTextureSwapChain before destroying the HMD with rev_Destroy.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
/// \note The \a format provided should be thought of as the format the distortion compositor will use when reading
/// the contents of the texture. To that end, it is highly recommended that the application requests texture swap chain
/// formats that are in sRGB-space (e.g. REV_FORMAT_R8G8B8A8_UNORM_SRGB) as the distortion compositor does sRGB-correct
/// rendering. Furthermore, the app should then make sure "glEnable(GL_FRAMEBUFFER_SRGB);" is called before rendering
/// into these textures. Even though it is not recommended, if the application would like to treat the texture as a linear
/// format and do linear-to-gamma conversion in GLSL, then the application can avoid calling "glEnable(GL_FRAMEBUFFER_SRGB);",
/// but should still pass in an sRGB variant for the \a format. Failure to do so will cause the distortion compositor
/// to apply incorrect gamma conversions leading to gamma-curve artifacts.
///
/// \see rev_GetTextureSwapChainLength
/// \see rev_GetTextureSwapChainCurrentIndex
/// \see rev_GetTextureSwapChainDesc
/// \see rev_GetTextureSwapChainBufferGL
/// \see rev_DestroyTextureSwapChain
///
REV_PUBLIC_FUNCTION(revResult) rev_CreateTextureSwapChainGL(revSession session,
                                                            const revTextureSwapChainDesc* desc,
                                                            revTextureSwapChain* out_TextureSwapChain);

/// Get a specific buffer within the chain as a GL texture name
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  chain Specifies an revTextureSwapChain previously returned by rev_CreateTextureSwapChainGL
/// \param[in]  index Specifies the index within the chain to retrieve. Must be between 0 and length (see rev_GetTextureSwapChainLength)
///             or may pass -1 to get the buffer at the CurrentIndex location. (Saving a call to GetTextureSwapChainCurrentIndex)
/// \param[out] out_TexId Returns the GL texture object name associated with the specific index requested
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainBufferGL(revSession session,
                                                               revTextureSwapChain chain,
                                                               int index,
                                                               unsigned int* out_TexId);


/// Creates a Mirror Texture which is auto-refreshed to mirror Rift contents produced by this application.
///
/// A second call to rev_CreateMirrorTextureGL for a given revSession before destroying the first one
/// is not supported and will result in an error return.
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  desc Specifies the requested mirror texture description.
/// \param[out] out_MirrorTexture Specifies the created revMirrorTexture, which will be valid upon a successful return value, else it will be NULL.
///             This texture must be eventually destroyed via rev_DestroyMirrorTexture before destroying the HMD with rev_Destroy.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
/// \note The \a format provided should be thought of as the format the distortion compositor will use when writing into the mirror
/// texture. It is highly recommended that mirror textures are requested as sRGB formats because the distortion compositor
/// does sRGB-correct rendering. If the application requests a non-sRGB format (e.g. R8G8B8A8_UNORM) as the mirror texture,
/// then the application might have to apply a manual linear-to-gamma conversion when reading from the mirror texture.
/// Failure to do so can result in incorrect gamma conversions leading to gamma-curve artifacts and color banding.
///
/// \see rev_GetMirrorTextureBufferGL
/// \see rev_DestroyMirrorTexture
///
REV_PUBLIC_FUNCTION(revResult) rev_CreateMirrorTextureGL(revSession session,
                                                         const revMirrorTextureDesc* desc,
                                                         revMirrorTexture* out_MirrorTexture);

/// Get a the underlying buffer as a GL texture name
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  mirrorTexture Specifies an revMirrorTexture previously returned by rev_CreateMirrorTextureGL
/// \param[out] out_TexId Specifies the GL texture object name associated with the mirror texture
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use 
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetMirrorTextureBufferGL(revSession session,
                                                            revMirrorTexture mirrorTexture,
                                                            unsigned int* out_TexId);


#endif    // REV_CAPI_GL_h
