#ifndef __IMGUI_SHADER_BLOBS_INCLUDED__
#define __IMGUI_SHADER_BLOBS_INCLUDED__

struct shader_t
{
    const void* shaderBlob;
    unsigned int shaderBlobSize;
};

#ifdef __cplusplus
extern "C" {
#endif

shader_t getDX10VertexShader();
shader_t getDX11VertexShader(unsigned long feature_level);
shader_t getDX12VertexShader();

shader_t getDX10PixelShader();
shader_t getDX11PixelShader(unsigned long feature_level);
shader_t getDX12PixelShader();

#ifdef __cplusplus
}
#endif

#endif
