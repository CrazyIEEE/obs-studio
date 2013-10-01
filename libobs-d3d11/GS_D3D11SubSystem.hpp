/******************************************************************************
    Copyright (C) 2013 by Hugh Bailey <obs.jim@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#pragma once

#include <vector>
#include <string>

#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "util/base.h"
#include "graphics/matrix4.h"
#include "graphics/graphics.h"
#include "util/windows/ComPtr.hpp"
#include "util/windows/HRError.hpp"

struct shader_var;
struct shader_sampler;
struct gs_vertex_shader;

#include "GS_D3D11Exports.h"

using namespace std;

/*
 * Just to clarify, all structs, and all public.  These are exporting only
 * via encapsulated C bindings, not C++ bindings, so the whole concept of
 * "public" and "private" does not matter at all for this subproject.
 */

#define MAX_TEXTURES 8

static inline uint32_t GetWinVer()
{
	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	return (ovi.dwMajorVersion << 8) | (ovi.dwMinorVersion);
}

static inline uint32_t GetFormatBPP(gs_color_format format)
{
	switch (format)
	{
	case GS_A8:          return 1;
	case GS_R8:          return 1;
	case GS_RGBA:        return 4;
	case GS_BGRX:        return 4;
	case GS_BGRA:        return 4;
	case GS_R10G10B10A2: return 4;
	case GS_RGBA16:      return 8;
	case GS_R16:         return 2;
	case GS_RGBA16F:     return 8;
	case GS_RGBA32F:     return 16;
	case GS_RG16F:       return 4;
	case GS_RG32F:       return 8;
	case GS_R16F:        return 2;
	case GS_R32F:        return 4;
	case GS_DXT1:        return 0;
	case GS_DXT3:        return 0;
	case GS_DXT5:        return 0;
	default:             return 0;
	}
}

static inline DXGI_FORMAT ConvertGSTextureFormat(gs_color_format format)
{
	switch (format)
	{
	case GS_A8:          return DXGI_FORMAT_A8_UNORM;
	case GS_R8:          return DXGI_FORMAT_R8_UNORM;
	case GS_RGBA:        return DXGI_FORMAT_R8G8B8A8_UNORM;
	case GS_BGRX:        return DXGI_FORMAT_B8G8R8X8_UNORM;
	case GS_BGRA:        return DXGI_FORMAT_B8G8R8A8_UNORM;
	case GS_R10G10B10A2: return DXGI_FORMAT_R10G10B10A2_UNORM;
	case GS_RGBA16:      return DXGI_FORMAT_R16G16B16A16_UNORM;
	case GS_R16:         return DXGI_FORMAT_R16_UNORM;
	case GS_RGBA16F:     return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case GS_RGBA32F:     return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case GS_RG16F:       return DXGI_FORMAT_R16G16_FLOAT;
	case GS_RG32F:       return DXGI_FORMAT_R32G32_FLOAT;
	case GS_R16F:        return DXGI_FORMAT_R16_FLOAT;
	case GS_R32F:        return DXGI_FORMAT_R32_FLOAT;
	case GS_DXT1:        return DXGI_FORMAT_BC1_UNORM;
	case GS_DXT3:        return DXGI_FORMAT_BC2_UNORM;
	case GS_DXT5:        return DXGI_FORMAT_BC3_UNORM;
	default:             return DXGI_FORMAT_UNKNOWN;
	}
}

static inline DXGI_FORMAT ConvertGSZStencilFormat(gs_zstencil_format format)
{
	switch (format) {
	case GS_Z16:         return DXGI_FORMAT_D16_UNORM;
	case GS_Z24_S8:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case GS_Z32F:        return DXGI_FORMAT_D32_FLOAT;
	case GS_Z32F_S8X24:  return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	default:             return DXGI_FORMAT_UNKNOWN;
	}
}

static inline D3D11_COMPARISON_FUNC ConvertGSDepthTest(gs_depth_test test)
{
	switch (test) {
	default:
	case GS_NEVER:    return D3D11_COMPARISON_NEVER;
	case GS_LESS:     return D3D11_COMPARISON_LESS;
	case GS_LEQUAL:   return D3D11_COMPARISON_LESS_EQUAL;
	case GS_EQUAL:    return D3D11_COMPARISON_EQUAL;
	case GS_GEQUAL:   return D3D11_COMPARISON_GREATER_EQUAL;
	case GS_GREATER:  return D3D11_COMPARISON_GREATER;
	case GS_NOTEQUAL: return D3D11_COMPARISON_NOT_EQUAL;
	case GS_ALWAYS:   return D3D11_COMPARISON_ALWAYS;
	}
}

static inline D3D11_STENCIL_OP ConvertGSStencilOp(gs_stencil_op op)
{
	switch (op) {
	default:
	case GS_KEEP:    return D3D11_STENCIL_OP_KEEP;
	case GS_ZERO:    return D3D11_STENCIL_OP_ZERO;
	case GS_REPLACE: return D3D11_STENCIL_OP_REPLACE;
	case GS_INCR:    return D3D11_STENCIL_OP_INCR;
	case GS_DECR:    return D3D11_STENCIL_OP_DECR;
	case GS_INVERT:  return D3D11_STENCIL_OP_INVERT;
	}
}

static inline D3D11_BLEND ConvertGSBlendType(gs_blend_type type)
{
	switch (type) {
	default:
	case GS_BLEND_ZERO:        return D3D11_BLEND_ZERO;
	case GS_BLEND_ONE:         return D3D11_BLEND_ONE;
	case GS_BLEND_SRCCOLOR:    return D3D11_BLEND_SRC_COLOR;
	case GS_BLEND_INVSRCCOLOR: return D3D11_BLEND_INV_SRC_COLOR;
	case GS_BLEND_SRCALPHA:    return D3D11_BLEND_SRC_ALPHA;
	case GS_BLEND_INVSRCALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
	case GS_BLEND_DSTCOLOR:    return D3D11_BLEND_DEST_COLOR;
	case GS_BLEND_INVDSTCOLOR: return D3D11_BLEND_INV_DEST_COLOR;
	case GS_BLEND_DSTALPHA:    return D3D11_BLEND_DEST_ALPHA;
	case GS_BLEND_INVDSTALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
	case GS_BLEND_SRCALPHASAT: return D3D11_BLEND_SRC_ALPHA_SAT;
	}
}

static inline D3D11_CULL_MODE ConvertGSCullMode(gs_cull_mode mode)
{
	switch (mode) {
	default:
	case GS_BACK:    return D3D11_CULL_BACK;
	case GS_FRONT:   return D3D11_CULL_FRONT;
	case GS_NEITHER: return D3D11_CULL_NONE;
	}
}

static inline D3D11_PRIMITIVE_TOPOLOGY ConvertGSTopology(gs_draw_mode mode)
{
	switch (mode) {
	default:
	case GS_POINTS:    return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case GS_LINES:     return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case GS_LINESTRIP: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case GS_TRIS:      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case GS_TRISTRIP:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	}
}

/* exception-safe RAII wrapper for vertex buffer data (NOTE: not copy-safe) */
struct VBDataPtr {
	vb_data *data;

	inline VBDataPtr(vb_data *data) : data(data) {}
	inline ~VBDataPtr() {vbdata_destroy(data);}
};

struct gs_vertex_buffer {
	ComPtr<ID3D11Buffer>         vertexBuffer;
	ComPtr<ID3D11Buffer>         normalBuffer;
	ComPtr<ID3D11Buffer>         colorBuffer;
	ComPtr<ID3D11Buffer>         tangentBuffer;
	vector<ComPtr<ID3D11Buffer>> uvBuffers;

	device_t       device;
	bool           dynamic;
	VBDataPtr      vbd;
	size_t         numVerts;
	vector<size_t> uvSizes;

	void FlushBuffer(ID3D11Buffer *buffer, void *array,
			size_t elementSize);

	void MakeBufferList(gs_vertex_shader *shader,
			vector<ID3D11Buffer*> &buffers,
			vector<uint32_t> &strides);

	inline void InitBuffer(const size_t elementSize,
			const size_t numVerts, void *array,
			ID3D11Buffer **buffer);

	gs_vertex_buffer(device_t device, struct vb_data *data, uint32_t flags);
};

/* exception-safe RAII wrapper for index buffer data (NOTE: not copy-safe) */
struct DataPtr {
	void *data;

	inline DataPtr(void *data) : data(data) {}
	inline ~DataPtr() {bfree(data);}
};

struct gs_index_buffer {
	ComPtr<ID3D11Buffer> indexBuffer;
	device_t             device;
	bool                 dynamic;
	gs_index_type        type;
	size_t               indexSize;
	size_t               num;
	DataPtr              indices;

	void InitBuffer();

	gs_index_buffer(device_t device, enum gs_index_type type,
			void *indices, size_t num, uint32_t flags);
};

struct gs_texture {
	gs_texture_type type;
	ComPtr<ID3D11ShaderResourceView> shaderRes;
	gs_device *device;

	inline gs_texture(gs_device *device, gs_texture_type type)
		: device (device),
		  type   (type)
	{
	}

	virtual ~gs_texture() {}
};

struct gs_texture_2d : gs_texture {
	ComPtr<ID3D11Texture2D>          texture;
	ComPtr<ID3D11RenderTargetView>   renderTarget[6];
	ComPtr<IDXGISurface1>            gdiSurface;

	uint32_t        width, height;
	gs_color_format format;
	DXGI_FORMAT     dxgiFormat;
	bool            isRenderTarget;
	bool            isGDICompatible;
	bool            isDynamic;
	bool            isShared;
	bool            genMipmaps;
	HANDLE          sharedHandle;

	void InitSRD(D3D11_SUBRESOURCE_DATA *srd, void *data);
	void InitTexture(void *data);
	void InitResourceView();
	void InitRenderTargets();

	inline gs_texture_2d()
		: gs_texture      (NULL, GS_TEXTURE_2D),
		  width           (0),
		  height          (0),
		  format          (GS_UNKNOWN),
		  dxgiFormat      (DXGI_FORMAT_UNKNOWN),
		  isRenderTarget  (false),
		  isGDICompatible (false),
		  isDynamic       (false),
		  isShared        (false),
		  genMipmaps      (false),
		  sharedHandle    (NULL)
	{
	}

	gs_texture_2d(device_t device, uint32_t width, uint32_t height,
			gs_color_format colorFormat, void *data,
			uint32_t flags, bool isCubeMap, bool gdiCompatible,
			bool shared);
};

struct gs_zstencil_buffer {
	ComPtr<ID3D11Texture2D>        texture;
	ComPtr<ID3D11DepthStencilView> view;

	gs_device          *device;
	uint32_t           width, height;
	gs_zstencil_format format;
	DXGI_FORMAT        dxgiFormat;

	void InitBuffer();

	inline gs_zstencil_buffer()
		: device     (NULL),
		  width      (0),
		  height     (0),
		  dxgiFormat (DXGI_FORMAT_UNKNOWN)
	{
	}

	gs_zstencil_buffer(device_t device, uint32_t width, uint32_t height,
			gs_zstencil_format format);
};

struct gs_stage_surface {
	ComPtr<ID3D11Texture2D> texture;

	gs_device       *device;
	uint32_t        width, height;
	gs_color_format format;
	DXGI_FORMAT     dxgiFormat;

	gs_stage_surface(device_t device, uint32_t width, uint32_t height,
			gs_color_format colorFormat);
};

struct gs_sampler_state {
	ComPtr<ID3D11SamplerState> state;
	device_t                   device;
	gs_sampler_info            info;

	gs_sampler_state(device_t device, gs_sampler_info *info);
};

struct shader_param {
	string            name;
	shader_param_type type;

	uint32_t textureID;

	int arrayCount;

	vector<uint8_t> curValue;
	vector<uint8_t> defaultValue;
	bool            changed;

	shader_param(shader_var &var, uint32_t &texCounter);
};

struct ShaderError {
	ComPtr<ID3D10Blob> errors;
	HRESULT hr;

	inline ShaderError(const ComPtr<ID3D10Blob> &errors, HRESULT hr)
		: errors (errors),
		  hr     (hr)
	{
	}
};

struct gs_shader {
	device_t             device;
	shader_type          type;
	vector<shader_param> params;
	ComPtr<ID3D11Buffer> constants;
	size_t               constantSize;

	inline void UpdateParam(vector<uint8_t> &constData, shader_param &param,
			bool &upload);
	void UploadParams();

	void BuildConstantBuffer();
	void Compile(const char *shaderStr, const char *file,
			const char *target, ID3D10Blob **shader);

	inline gs_shader(device_t device, shader_type type)
		: device       (device),
		  type         (type),
		  constantSize (0)
	{
	}

	virtual ~gs_shader() {}
};

struct ShaderSampler {
	string           name;
	gs_sampler_state sampler;

	inline ShaderSampler(const char *name, device_t device,
			gs_sampler_info *info)
		: name    (name),
		  sampler (device, info)
	{
	}
};

struct gs_vertex_shader : gs_shader {
	ComPtr<ID3D11VertexShader> shader;
	ComPtr<ID3D11InputLayout>  layout;

	shader_param *world, *viewProj;

	bool     hasNormals;
	bool     hasColors;
	bool     hasTangents;
	uint32_t nTexUnits;

	inline uint32_t NumBuffersExpected() const
	{
		uint32_t count = nTexUnits+1;
		if (hasNormals)  count++;
		if (hasColors)   count++;
		if (hasTangents) count++;

		return count;
	}

	void GetBuffersExpected(const vector<D3D11_INPUT_ELEMENT_DESC> &inputs);

	gs_vertex_shader(device_t device, const char *file,
			const char *shaderString);
};

struct gs_pixel_shader : gs_shader {
	ComPtr<ID3D11PixelShader> shader;
	vector<ShaderSampler>     samplers;

	inline void GetSamplerStates(ID3D11SamplerState **states)
	{
		size_t i;
		for (i = 0; i < samplers.size(); i++)
			states[i] = samplers[i].sampler.state;
		for (; i < MAX_TEXTURES; i++)
			states[i] = NULL;
	}

	gs_pixel_shader(device_t device, const char *file,
			const char *shaderString);
};

struct gs_swap_chain {
	gs_device                      *device;
	uint32_t                       numBuffers;
	HWND                           hwnd;

	gs_texture_2d                  target;
	gs_zstencil_buffer             zs;
	ComPtr<IDXGISwapChain>         swap;

	void InitTarget(uint32_t cx, uint32_t cy);
	void InitZStencilBuffer(uint32_t cx, uint32_t cy);
	void Resize(uint32_t cx, uint32_t cy);
	void Init(gs_init_data *data);

	inline gs_swap_chain()
		: device     (NULL),
		  numBuffers (0),
		  hwnd       (NULL)
	{
	}

	gs_swap_chain(gs_device *device, gs_init_data *data);
};

struct BlendState {
	bool          blendEnabled;
	gs_blend_type srcFactor;
	gs_blend_type destFactor;

	bool          redEnabled;
	bool          greenEnabled;
	bool          blueEnabled;
	bool          alphaEnabled;

	inline BlendState()
		: blendEnabled (true),
		  srcFactor    (GS_BLEND_SRCALPHA),
		  destFactor   (GS_BLEND_INVSRCALPHA),
		  redEnabled   (true),
		  greenEnabled (true),
		  blueEnabled  (true),
		  alphaEnabled (true)
	{
	}

	inline BlendState(const BlendState &state)
	{
		memcpy(this, &state, sizeof(BlendState));
	}
};

struct SavedBlendState : BlendState {
	ComPtr<ID3D11BlendState> state;

	inline SavedBlendState(const BlendState &val) : BlendState(val)
	{
	}
};

struct StencilSide {
	gs_depth_test test;
	gs_stencil_op fail;
	gs_stencil_op zfail;
	gs_stencil_op zpass;

	inline StencilSide()
		: test  (GS_ALWAYS),
		  fail  (GS_KEEP),
		  zfail (GS_KEEP),
		  zpass (GS_KEEP)
	{
	}
};

struct ZStencilState {
	bool          depthEnabled;
	bool          depthWriteEnabled;
	gs_depth_test depthFunc;

	bool          stencilEnabled;
	bool          stencilWriteEnabled;
	StencilSide   stencilFront;
	StencilSide   stencilBack;

	inline ZStencilState()
		: depthEnabled        (true),
		  depthWriteEnabled   (true),
		  depthFunc           (GS_LESS),
		  stencilEnabled      (false),
		  stencilWriteEnabled (true)
	{
	}

	inline ZStencilState(const ZStencilState &state)
	{
		memcpy(this, &state, sizeof(ZStencilState));
	}
};

struct SavedZStencilState : ZStencilState {
	ComPtr<ID3D11DepthStencilState> state;

	inline SavedZStencilState(const ZStencilState &val)
		: ZStencilState (val)
	{
	}
};

struct RasterState {
	gs_cull_mode cullMode;
	bool         scissorEnabled;

	inline RasterState()
		: cullMode       (GS_BACK),
		  scissorEnabled (false)
	{
	}

	inline RasterState(const RasterState &state)
	{
		memcpy(this, &state, sizeof(RasterState));
	}
};

struct SavedRasterState : RasterState {
	ComPtr<ID3D11RasterizerState> state;

	inline SavedRasterState(const RasterState &val)
	       : RasterState (val)
	{
	}
};

struct mat4float {
	float mat[16];
};

struct gs_device {
	ComPtr<IDXGIFactory1>       factory;
	ComPtr<ID3D11Device>        device;
	ComPtr<ID3D11DeviceContext> context;
	gs_swap_chain               defaultSwap;

	gs_texture_2d               *curRenderTarget;
	gs_zstencil_buffer          *curZStencilBuffer;
	int                         curRenderSide;
	gs_texture                  *curTextures[MAX_TEXTURES];
	gs_sampler_state            *curSamplers[MAX_TEXTURES];
	gs_vertex_buffer            *curVertexBuffer;
	gs_vertex_shader            *curVertexShader;
	gs_index_buffer             *curIndexBuffer;
	gs_pixel_shader             *curPixelShader;
	gs_swap_chain               *curSwapChain;

	bool                        zstencilStateChanged;
	bool                        rasterStateChanged;
	bool                        blendStateChanged;
	ZStencilState               zstencilState;
	RasterState                 rasterState;
	BlendState                  blendState;
	vector<SavedZStencilState>  zstencilStates;
	vector<SavedRasterState>    rasterStates;
	vector<SavedBlendState>     blendStates;
	ID3D11DepthStencilState     *curDepthStencilState;
	ID3D11RasterizerState       *curRasterState;
	ID3D11BlendState            *curBlendState;
	D3D11_PRIMITIVE_TOPOLOGY    curToplogy;

	gs_rect                     viewport;

	vector<mat4float>           projStack;

	matrix4                     curProjMatrix;
	matrix4                     curViewMatrix;
	matrix4                     curViewProjMatrix;

	void InitFactory(uint32_t adapterIdx, IDXGIAdapter1 **adapter);
	void InitDevice(gs_init_data *data, IDXGIAdapter *adapter);

	ID3D11DepthStencilState *AddZStencilState();
	ID3D11RasterizerState   *AddRasterState();
	ID3D11BlendState        *AddBlendState();
	void UpdateZStencilState();
	void UpdateRasterState();
	void UpdateBlendState();

	inline void CopyTex(ID3D11Texture2D *dst, texture_t src);

	void UpdateViewProjMatrix();

	gs_device(gs_init_data *data);
};
