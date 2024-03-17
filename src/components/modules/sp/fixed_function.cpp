#include "std_include.hpp"

#define USE_NEW_FX_SYS 0

using namespace game::sp;

namespace components::sp
{
	struct unpacked_model_vert
	{
		game::vec3_t pos;
		game::vec3_t normal;
		game::vec2_t texcoord;
	};

	constexpr auto MODEL_VERTEX_STRIDE = 32u;
	constexpr auto MODEL_VERTEX_FORMAT = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	// #

	struct unpacked_world_vert
	{
		game::vec3_t pos;
		game::vec3_t normal;
		unsigned int color;
		game::vec2_t texcoord;
	};

	constexpr auto WORLD_VERTEX_STRIDE = 36u;
	constexpr auto WORLD_VERTEX_FORMAT = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	IDirect3DVertexBuffer9* gfx_world_vertexbuffer = nullptr;

	// #

	static IDirect3DVertexBuffer9* dynamic_codemesh_vb = nullptr;

	// *
	// static models (rigid)

	bool XSurfaceOptimize(game::XSurface* surf)
	{
		const auto dev = game::sp::dx->device;
		bool allocated_any = false;

		// setup vertexbuffer
		{
			//XSurfaceOptimizeRigid(model, surf);
			if (surf && !surf->vb0 && surf->vertCount)
			{
				void* vertex_buffer_data = nullptr;
				const auto vertex_bytes = surf->vertCount * MODEL_VERTEX_STRIDE;

				if (auto hr = dev->CreateVertexBuffer(vertex_bytes, 8, 0, D3DPOOL_DEFAULT, &surf->vb0, nullptr);
					hr >= 0)
				{
					if (hr = surf->vb0->Lock(0, 0, &vertex_buffer_data, 0);
						hr >= 0)
					{
						// we need to unpack normal and texcoords in 'GfxPackedVertex' to be able to use them for fixed-function rendering
						for (auto i = 0; i < surf->vertCount; i++)
						{
							// packed source vertex
							const auto src_vert = surf->verts0[i];

							// position of our unpacked vert within the vertex buffer
							const auto v_pos_in_buffer = i * MODEL_VERTEX_STRIDE;
							const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)vertex_buffer_data + v_pos_in_buffer));

							// unpack normal and texcoords
							const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
							v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
							v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
							v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

							game::sp::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);

							// assign pos
							v->pos[0] = src_vert.xyz[0];
							v->pos[1] = src_vert.xyz[1];
							v->pos[2] = src_vert.xyz[2];
						}

						surf->vb0->Unlock();
					}
					else
					{
						surf->vb0->Release();
						surf->vb0 = nullptr;
					}

					allocated_any = true;
				}
			}
		}

		return allocated_any;
	}

	__declspec(naked) void XSurfaceOptimizeRigid_stub()
	{
		const static uint32_t retn_addr = 0x727D2F;
		__asm
		{
			pushad;
			push	edi; // xsurf
			call	XSurfaceOptimize;
			add		esp, 4;
			popad;

			jmp		retn_addr;
		}
	}

	int XModelGetSurfaces(const game::XModel* model, game::XSurface** surfaces, const int submodel)
	{
		*surfaces = &model->surfs[model->lodInfo[submodel].surfIndex];
		return model->lodInfo[submodel].numsurfs;
	}

	void XModelOptimize(const game::XModel* model)
	{
		game::XSurface* surfaces;

		const auto lod_count = model->numLods;
		for (auto lod_index = 0; lod_index < lod_count; ++lod_index)
		{
			const auto surf_count = XModelGetSurfaces(model, &surfaces, lod_index);
			for (auto surf_index = 0; surf_index < surf_count; ++surf_index)
			{
				if (XSurfaceOptimize(&surfaces[surf_index]))
				{
#ifdef DEBUG
					game::sp::Com_PrintMessage(0, utils::va("Allocated buffers for smodel '%s'\n", model->name), 0);
#endif
				}
			}
		}
	}

	/**
	 * @brief completely rewritten R_DrawStaticModelDrawSurfNonOptimized to render static models using the fixed-function pipeline
	 */
	void R_DrawStaticModelDrawSurfNonOptimized(const game::GfxStaticModelDrawStream* drawstream, [[maybe_unused]] game::GfxCmdBufSourceState* src, game::GfxCmdBufState* cmd)
	{
		const auto smodel_count = drawstream->smodelCount;
		const auto smodel_list = (const game::GfxStaticModelDrawStream*) reinterpret_cast<const void*>(drawstream->smodelList);
		const auto draw_inst = game::sp::rgp->world->dpvs.smodelDrawInsts;
		const auto dev = game::sp::dx->device;

		// create buffers for all surfaces of the model (including LODs)
		// ^ already done on map load (when 'rtx_warm_smodels' is enabled) but this also covers dynamically spawned models
		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];
			XModelOptimize(inst->model);
		}

		// #
		// set streams

		// index buffer
		if (cmd->prim.indexBuffer != drawstream->localSurf->indexBuffer)
		{
			cmd->prim.indexBuffer = drawstream->localSurf->indexBuffer;
			dev->SetIndices(drawstream->localSurf->indexBuffer);
		}

		// custom vertexbuffer
		if (drawstream->localSurf->vb0)
		{
			cmd->prim.streams[0].vb = drawstream->localSurf->vb0;
			cmd->prim.streams[0].offset = 0;
			cmd->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			dev->SetStreamSource(0, drawstream->localSurf->vb0, 0, MODEL_VERTEX_STRIDE);
		}
		else // fallback to shader rendering if there is no custom vertexbuffer
		{
			__debugbreak();
		}

		// vertex format
		dev->SetFVF(MODEL_VERTEX_FORMAT);

		// def. needed or the game will render the mesh using shaders
		dev->SetVertexShader(nullptr);
		dev->SetPixelShader(nullptr);

		// #
		// draw prim

		for (auto index = 0u; index < smodel_count; index++)
		{
			const auto inst = &draw_inst[*((std::uint16_t*)&smodel_list->primDrawSurfPos + index)];

			// transform model into the scene by updating the worldmatrix
			//R_DrawStaticModelDrawSurfPlacement(src, inst);

			float mtx[4][4] = {};
			const auto scale = inst->placement.scale;
			mtx[0][0] = inst->placement.axis[0][0] * scale;
			mtx[0][1] = inst->placement.axis[0][1] * scale;
			mtx[0][2] = inst->placement.axis[0][2] * scale;
			mtx[0][3] = 0.0f;

			mtx[1][0] = inst->placement.axis[1][0] * scale;
			mtx[1][1] = inst->placement.axis[1][1] * scale;
			mtx[1][2] = inst->placement.axis[1][2] * scale;
			mtx[1][3] = 0.0f;

			mtx[2][0] = inst->placement.axis[2][0] * scale;
			mtx[2][1] = inst->placement.axis[2][1] * scale;
			mtx[2][2] = inst->placement.axis[2][2] * scale;
			mtx[2][3] = 0.0f;

			mtx[3][0] = inst->placement.origin[0];
			mtx[3][1] = inst->placement.origin[1];
			mtx[3][2] = inst->placement.origin[2];
			mtx[3][3] = 1.0f;

			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

			// get indexbuffer offset
			const auto offset = 0;

			// draw the prim
			cmd->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, drawstream->localSurf->vertCount, offset, drawstream->localSurf->triCount);
		}

		dev->SetFVF(NULL);
	}

	// *
	// xmodels (rigid/skinned)

	game::GfxModelRigidSurface* saved_gfxmodel = nullptr;
	__declspec(naked) void save_gfxmodel_for_R_DrawXModelRigidModelSurf()
	{
		const static uint32_t retn_addr = 0x74AFCE;
		__asm
		{
			mov		saved_gfxmodel, ecx;

			// og code
			sub     esp, 0x10;
			mov     eax, esp;
			jmp		retn_addr;
		}
	}

	void R_DrawXModelRigidModelSurf(game::XSurface* unused_surf [[maybe_unused]], game::GfxCmdBufSourceState* source [[maybe_unused]], game::GfxCmdBufState* state)
	{
		if (!saved_gfxmodel || !state->material || !state->material->info.name)
		{
			__debugbreak();
			return;
		}

		const auto dev = game::sp::dx->device;
		const auto surf = saved_gfxmodel->surf.xsurf; 

		if ((surf->flags & 0x80))
		{
			__debugbreak();
		}

		if (!surf->vb0)
		{
			if (XSurfaceOptimize(surf) && state->material && state->material->info.name)
			{
#ifdef DEBUG
				game::sp::Com_PrintMessage(0, utils::va("Allocated buffers for smodel using material '%s'\n", state->material->info.name), 0);
#endif
			}
		}

		const auto current_material_name = std::string_view(state->material->info.name);

		// #
		// set streams

		state->prim.indexBuffer = surf->indexBuffer;
		state->prim.device->SetIndices(surf->indexBuffer);

		float custom_scalar = 1.0f;

		// skysphere materials need to have sort = sky in assetmanager
		if (state->material && state->material->info.sortKey == 5)
		{
			custom_scalar = 5.0f;

			// disable fog for skysphere
			state->prim.device->SetRenderState(D3DRS_FOGENABLE, FALSE);

			if (state->material->textureTable && state->material->textureTable->u.image)
			{
				if (state->material->textureTable->u.image->mapType == 5)
				{
					if (const auto identity = game::sp::gfxCmdBufSourceState->u.input.codeImages[game::TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP];
							identity && identity->texture.basemap)
					{
						// reduce rapid sky switching on some maps?
						if (fixed_function::last_valid_sky_texture)
						{
							game::sp::dx->device->SetTexture(0, fixed_function::last_valid_sky_texture);
						}
						else
						{
							game::sp::dx->device->SetTexture(0, identity->texture.basemap);
						}
					}
				}
				else
				{
					// non cubemap images or cubemaps that were replaced with kowloon
					game::sp::dx->device->SetTexture(0, state->material->textureTable->u.image->texture.basemap);
					fixed_function::last_valid_sky_texture = state->material->textureTable->u.image->texture.basemap;
				}

				/*main_module::setup_sky_image(state->material->textureTable->u.image);
				if (main_module::m_sky_texture)
				{
					game::sp::dx->device->SetTexture(0, main_module::m_sky_texture);
				}*/
			}
			/*if (const auto skyimg = reinterpret_cast<game::GfxImage**>(0x408930C); skyimg[0] && skyimg[0]->texture.basemap)
			{
				game::sp::dx->device->SetTexture(0, skyimg[0]->texture.basemap);
			}*/
		}

		// #
		// build world matrix

		float model_axis[3][3] = {};
		utils::unit_quat_to_axis(saved_gfxmodel->placement.base.quat, model_axis);

		float mtx[4][4] = {};
		const auto scale = saved_gfxmodel->placement.scale;

		mtx[0][0] = model_axis[0][0] * scale * custom_scalar;
		mtx[0][1] = model_axis[0][1] * scale * custom_scalar;
		mtx[0][2] = model_axis[0][2] * scale * custom_scalar;
		mtx[0][3] = 0.0f;

		mtx[1][0] = model_axis[1][0] * scale * custom_scalar;
		mtx[1][1] = model_axis[1][1] * scale * custom_scalar;
		mtx[1][2] = model_axis[1][2] * scale * custom_scalar;
		mtx[1][3] = 0.0f;

		mtx[2][0] = model_axis[2][0] * scale * custom_scalar;
		mtx[2][1] = model_axis[2][1] * scale * custom_scalar;
		mtx[2][2] = model_axis[2][2] * scale * custom_scalar;
		mtx[2][3] = 0.0f;

		mtx[3][0] = saved_gfxmodel->placement.base.origin[0];
		mtx[3][1] = saved_gfxmodel->placement.base.origin[1];
		mtx[3][2] = saved_gfxmodel->placement.base.origin[2];
		mtx[3][3] = 1.0f;

		if (surf->vb0)
		{
			// def. needed or the game will render the mesh using shaders
			dev->SetVertexShader(nullptr);
			dev->SetPixelShader(nullptr);

			state->prim.streams[0].vb = surf->vb0;
			state->prim.streams[0].offset = 0;
			state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			state->prim.device->SetStreamSource(0, surf->vb0, 0, MODEL_VERTEX_STRIDE);

			// vertex format
			dev->SetFVF(MODEL_VERTEX_FORMAT);

			// set world matrix
			dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));
		}
		
		// #
		// draw prim

		if (current_material_name == std::string_view("mc/mtl_p_rus_security_monitor_extracam"))
		{
			// extracam is currently rendered from player pov
			if (const auto extracam_rt = reinterpret_cast<game::GfxRenderTarget*>(0x45EB3A0); 
				extracam_rt && extracam_rt->image && extracam_rt->image->texture.basemap)
			{
				game::sp::dx->device->SetTexture(0, extracam_rt->image->texture.basemap);
			}
		}

//#define CHANGE_CINE_COLOR
#ifdef CHANGE_CINE_COLOR
		DWORD saved_st0_colorop = 0, saved_st0_colorarg1 = 0, saved_st0_colorarg2 = 0;
		DWORD saved_st1_colorop = 0, saved_st1_colorarg1 = 0, saved_st1_colorarg2 = 0;
		DWORD saved_texfactor = 0;
		bool changed_states = false;
#endif

		// tv with anim
		if (current_material_name == std::string_view("mc/mtl_bink_4x4"))
		{
			if (const auto cinematic_y = game::sp::gfxCmdBufSourceState->u.input.codeImages[game::TEXTURE_SRC_CODE_CINEMATIC_Y];
				cinematic_y && cinematic_y->texture.basemap)
			{
				// mutlitexture blending is not supported by remix?
				// https://stackoverflow.com/questions/4041840/function-to-convert-ycbcr-to-rgb

				game::sp::dx->device->SetTexture(0, cinematic_y->texture.basemap);

#ifdef CHANGE_CINE_COLOR
				if (!game::is_game_mod)
				{
					changed_states = true;
				
					game::sp::dx->device->GetTextureStageState(0, D3DTSS_COLOROP, &saved_st0_colorop);
					game::sp::dx->device->GetTextureStageState(0, D3DTSS_COLORARG1, &saved_st0_colorarg1);
					game::sp::dx->device->GetTextureStageState(0, D3DTSS_COLORARG2, &saved_st0_colorarg2);
					game::sp::dx->device->GetTextureStageState(1, D3DTSS_COLOROP, &saved_st1_colorop);
					game::sp::dx->device->GetTextureStageState(1, D3DTSS_COLORARG1, &saved_st1_colorarg1);
					game::sp::dx->device->GetTextureStageState(1, D3DTSS_COLORARG2, &saved_st1_colorarg2);
					game::sp::dx->device->GetRenderState(D3DRS_TEXTUREFACTOR, &saved_texfactor);

					game::sp::dx->device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					game::sp::dx->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

					game::sp::dx->device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_XRGB(174, 220, 184));
					game::sp::dx->device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
					game::sp::dx->device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
					game::sp::dx->device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
				}
#endif
			}
		}

		if (current_material_name == std::string_view("mc/mtl_ui3d_0") 
			|| current_material_name == std::string_view("mc/mtl_ui3d_1"))
		{
			if (const auto ui3d_tex = game::sp::gfxCmdBufSourceState->u.input.codeImages[game::TEXTURE_SRC_CODE_UI3D];
				ui3d_tex && ui3d_tex->texture.basemap)
			{
				game::sp::dx->device->SetTexture(0, ui3d_tex->texture.basemap);
			}
		}

		const auto offset = 0;
		state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surf->vertCount, offset, surf->triCount);
		dev->SetFVF(NULL);

#ifdef CHANGE_CINE_COLOR
		// restore states if changed
		if (changed_states && !game::is_game_mod)
		{
			game::sp::dx->device->SetTextureStageState(0, D3DTSS_COLOROP, saved_st0_colorop);
			game::sp::dx->device->SetTextureStageState(0, D3DTSS_COLORARG1, saved_st0_colorarg1);
			game::sp::dx->device->SetTextureStageState(0, D3DTSS_COLORARG2, saved_st0_colorarg2);
			game::sp::dx->device->SetTextureStageState(1, D3DTSS_COLOROP, saved_st1_colorop);
			game::sp::dx->device->SetTextureStageState(1, D3DTSS_COLORARG1, saved_st1_colorarg1);
			game::sp::dx->device->SetTextureStageState(1, D3DTSS_COLORARG2, saved_st1_colorarg2);
			game::sp::dx->device->SetRenderState(D3DRS_TEXTUREFACTOR, saved_texfactor);
		}
#endif
	}

	// ------------------------

	int R_SetIndexData(game::GfxCmdBufPrimState* state, const unsigned __int16* indices, int tri_count)
	{
		const auto buf = game::sp::gfx_buf;
		const auto index_count = 3 * tri_count;

		if (index_count + buf->dynamicIndexBuffer->used > buf->dynamicIndexBuffer->total)
		{
			buf->dynamicIndexBuffer->used = 0;
		}

		if (!buf->dynamicIndexBuffer->used)
		{
			buf->dynamicIndexBuffer = buf->dynamicIndexBufferPool;
		}

		const auto base_index = buf->dynamicIndexBuffer->used;

		void* buffer_data;
		if (const auto hr = buf->dynamicIndexBuffer->buffer->Lock(
			2 * buf->dynamicIndexBuffer->used,
			6 * tri_count, 
			&buffer_data, 
			buf->dynamicIndexBuffer->used != 0 ? 0x1000 : 0x2000);
			hr < 0)
		{
			//R_FatalLockError(hr);
			//game::sp::Com_Error(0, "Fatal lock error :: R_SetIndexData");
			__debugbreak();
			return -1;
		}

		if (buffer_data)
		{
			memcpy(buffer_data, indices, 2 * index_count);
		}

		buf->dynamicIndexBuffer->buffer->Unlock();

		if (state->indexBuffer != buf->dynamicIndexBuffer->buffer)
		{
			state->indexBuffer = buf->dynamicIndexBuffer->buffer;
			state->device->SetIndices(buf->dynamicIndexBuffer->buffer);
		}

		if (buffer_data)
		{
			buf->dynamicIndexBuffer->used += index_count;
		}

		return base_index;
	}

	void R_DrawXModelSkinnedUncached(game::GfxModelRigidSurface* skinned_surf, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state, int is_skinned_vert)
	{
		// fixed function rendering

		const auto surf = skinned_surf->surf.xsurf;
		const auto start_index = R_SetIndexData(&state->prim, surf->triIndices, surf->triCount);

		if ((int)(MODEL_VERTEX_STRIDE * surf->vertCount + game::sp::gfx_buf->dynamicVertexBuffer->used) > game::sp::gfx_buf->dynamicVertexBuffer->total)
		{
			game::sp::gfx_buf->dynamicVertexBuffer->used = 0;
		}

		// R_SetVertexData
		void* buffer_data;
		if (const auto hr = game::sp::gfx_buf->dynamicVertexBuffer->buffer->Lock(game::sp::gfx_buf->dynamicVertexBuffer->used, MODEL_VERTEX_STRIDE * surf->vertCount, &buffer_data, game::sp::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000);
			hr < 0)
		{
			//R_FatalLockError(hr);
			//game::sp::Com_Error(0, "Fatal lock error :: R_DrawXModelSkinnedUncached");
			__debugbreak();
		}

		{
			for (auto i = 0u; i < surf->vertCount; i++)
			{
				// packed source vertex
				const auto src_vert = is_skinned_vert ? skinned_surf->surf.u.skinnedVert[i] : *surf->verts0;

				// position of our unpacked vert within the vertex buffer
 				const auto v_pos_in_buffer = i * MODEL_VERTEX_STRIDE;
				const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

				// vert pos
				v->pos[0] = src_vert.xyz[0];
				v->pos[1] = src_vert.xyz[1];
				v->pos[2] = src_vert.xyz[2];

				// unpack and assign vert normal

				// normal unpacking in a cod4 hlsl shader:
				// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
				// temp0.xyz = temp0.www * temp0;

				const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
				v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
				v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
				v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

				// uv's
				game::sp::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);
			}

		}

		game::sp::gfx_buf->dynamicVertexBuffer->buffer->Unlock();
		const std::uint32_t vert_offset = game::sp::gfx_buf->dynamicVertexBuffer->used;
		game::sp::gfx_buf->dynamicVertexBuffer->used += (MODEL_VERTEX_STRIDE * surf->vertCount);


		// #
		// #

		if (state->prim.streams[0].vb != game::sp::gfx_buf->dynamicVertexBuffer->buffer || state->prim.streams[0].offset != vert_offset || state->prim.streams[0].stride != MODEL_VERTEX_STRIDE)
		{
			state->prim.streams[0].vb = game::sp::gfx_buf->dynamicVertexBuffer->buffer;
			state->prim.streams[0].offset = vert_offset;
			state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
			state->prim.device->SetStreamSource(0, game::sp::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, MODEL_VERTEX_STRIDE);
		}

		// needed or game renders mesh with shaders
		state->prim.device->SetVertexShader(nullptr);
		state->prim.device->SetPixelShader(nullptr);

		// vertex format
		state->prim.device->SetFVF(MODEL_VERTEX_FORMAT); 


		// #
		// build world matrix

		float model_axis[3][3] = {};
		utils::unit_quat_to_axis(is_skinned_vert ? src->skinnedPlacement.base.quat : skinned_surf->placement.base.quat /*src->objectPlacement->base.quat*/, model_axis);

		//const auto mtx = source->matrices.matrix[0].m;
		float mtx[4][4] = {};
		const auto scale = is_skinned_vert ? src->skinnedPlacement.scale : skinned_surf->placement.scale /*src->objectPlacement->scale*/;

		// inlined ikMatrixSet44
		mtx[0][0] = model_axis[0][0] * scale;
		mtx[0][1] = model_axis[0][1] * scale;
		mtx[0][2] = model_axis[0][2] * scale;
		mtx[0][3] = 0.0f;

		mtx[1][0] = model_axis[1][0] * scale;
		mtx[1][1] = model_axis[1][1] * scale;
		mtx[1][2] = model_axis[1][2] * scale;
		mtx[1][3] = 0.0f;

		mtx[2][0] = model_axis[2][0] * scale;
		mtx[2][1] = model_axis[2][1] * scale;
		mtx[2][2] = model_axis[2][2] * scale;
		mtx[2][3] = 0.0f;

		mtx[3][0] = is_skinned_vert ? src->skinnedPlacement.base.origin[0] : skinned_surf->placement.base.origin[0]; //src->objectPlacement->base.origin[0];
		mtx[3][1] = is_skinned_vert ? src->skinnedPlacement.base.origin[1] : skinned_surf->placement.base.origin[1]; //src->objectPlacement->base.origin[1];
		mtx[3][2] = is_skinned_vert ? src->skinnedPlacement.base.origin[2] : skinned_surf->placement.base.origin[2]; //src->objectPlacement->base.origin[2];
		mtx[3][3] = 1.0f;

		// set world matrix
		state->prim.device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));
		state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surf->vertCount, start_index, surf->triCount);
		state->prim.device->SetFVF(NULL);
	}

	__declspec(naked) void R_DrawXModelSkinnedUncached_stub()
	{
		const static uint32_t retn_addr = 0x73C988;
		__asm
		{
			push	1;
			push    edi; // state
			push    ebx; // source
			push	esi; // GfxModelSkinnedSurface
			call	R_DrawXModelSkinnedUncached;
			add		esp, 16;
			jmp		retn_addr;
		}
	}

	__declspec(naked) void R_DrawXModelSkinnedUncached_stub2()
	{
		const static uint32_t retn_addr = 0x73DFB8;
		__asm
		{
			push	0;
			push    edi; // state
			push    ebx; // source
			push	eax; // GfxModelSkinnedSurface
			call	R_DrawXModelSkinnedUncached;
			add		esp, 4;
			jmp		retn_addr;
		}
	}

	void R_DrawStaticModelsSkinnedDrawSurf(game::GfxStaticModelDrawStream* draw_stream, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state)
	{
		const auto surf = draw_stream->localSurf;
		const auto start_index = R_SetIndexData(&state->prim, surf->triIndices, surf->triCount);

		/*if (!surf->deformed && surf->custom_vertexbuffer)
		{
			if (state->prim.streams[0].vb != surf->custom_vertexbuffer || state->prim.streams[0].offset != 0 || state->prim.streams[0].stride != MODEL_VERTEX_STRIDE)
			{
				state->prim.streams[0].vb = surf->custom_vertexbuffer;
				state->prim.streams[0].offset = 0;
				state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
				state->prim.device->SetStreamSource(0, surf->custom_vertexbuffer, 0, MODEL_VERTEX_STRIDE);
			}
		}
		else*/
		{
			if ((int)(MODEL_VERTEX_STRIDE * surf->vertCount + game::sp::gfx_buf->dynamicVertexBuffer->used) > game::sp::gfx_buf->dynamicVertexBuffer->total)
			{
				game::sp::gfx_buf->dynamicVertexBuffer->used = 0;
			}

			// R_SetVertexData
			void* buffer_data;
			if (const auto hr = game::sp::gfx_buf->dynamicVertexBuffer->buffer->Lock(game::sp::gfx_buf->dynamicVertexBuffer->used, MODEL_VERTEX_STRIDE * surf->vertCount, &buffer_data, game::sp::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000);
				hr < 0)
			{
				//R_FatalLockError(hr);
				//game::Com_Error(game::ERR_DROP, "Fatal lock error :: R_DrawXModelSkinnedUncached");
				__debugbreak();
			}
			{
				for (auto i = 0u; i < surf->vertCount; i++)
				{
					// packed source vertex
					const auto src_vert = surf->verts0[i];

					// position of our unpacked vert within the vertex buffer
					const auto v_pos_in_buffer = i * MODEL_VERTEX_STRIDE;
					const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

					// vert pos
					v->pos[0] = src_vert.xyz[0];
					v->pos[1] = src_vert.xyz[1];
					v->pos[2] = src_vert.xyz[2];

					// unpack and assign vert normal

					// normal unpacking in a cod4 hlsl shader:
					// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
					// temp0.xyz = temp0.www * temp0;

					const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
					v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
					v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
					v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

					// uv's
					game::sp::Vec2UnpackTexCoords(src_vert.texCoord.packed, v->texcoord);
				}
			}

			game::sp::gfx_buf->dynamicVertexBuffer->buffer->Unlock();
			const std::uint32_t vert_offset = game::sp::gfx_buf->dynamicVertexBuffer->used;
			game::sp::gfx_buf->dynamicVertexBuffer->used += (MODEL_VERTEX_STRIDE * surf->vertCount);

			// #
			// #

			if (state->prim.streams[0].vb != game::sp::gfx_buf->dynamicVertexBuffer->buffer || state->prim.streams[0].offset != vert_offset || state->prim.streams[0].stride != MODEL_VERTEX_STRIDE)
			{
				state->prim.streams[0].vb = game::sp::gfx_buf->dynamicVertexBuffer->buffer;
				state->prim.streams[0].offset = vert_offset;
				state->prim.streams[0].stride = MODEL_VERTEX_STRIDE;
				state->prim.device->SetStreamSource(0, game::sp::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, MODEL_VERTEX_STRIDE);
			}
		}

		{
			// needed or game renders mesh with shaders
			state->prim.device->SetVertexShader(nullptr);
			state->prim.device->SetPixelShader(nullptr);

			// vertex format
			state->prim.device->SetFVF(MODEL_VERTEX_FORMAT);

			// #
			// build world matrix

			for (auto index = 0u; index < draw_stream->smodelCount; index++)
			{
				const auto inst = &game::sp::rgp->world->dpvs.smodelDrawInsts[draw_stream->smodelList[index]];

				float mtx[4][4] = {};
				const auto scale = src->skinnedPlacement.scale;

				// inlined ikMatrixSet44
				mtx[0][0] = inst->placement.axis[0][0] * scale;
				mtx[0][1] = inst->placement.axis[0][1] * scale;
				mtx[0][2] = inst->placement.axis[0][2] * scale;
				mtx[0][3] = 0.0f;

				mtx[1][0] = inst->placement.axis[1][0] * scale;
				mtx[1][1] = inst->placement.axis[1][1] * scale;
				mtx[1][2] = inst->placement.axis[1][2] * scale;
				mtx[1][3] = 0.0f;

				mtx[2][0] = inst->placement.axis[2][0] * scale;
				mtx[2][1] = inst->placement.axis[2][1] * scale;
				mtx[2][2] = inst->placement.axis[2][2] * scale;
				mtx[2][3] = 0.0f;

				mtx[3][0] = inst->placement.origin[0];
				mtx[3][1] = inst->placement.origin[1];
				mtx[3][2] = inst->placement.origin[2];
				mtx[3][3] = 1.0f;

				// set world matrix
				state->prim.device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

				// draw the prim
				state->prim.device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, surf->vertCount, start_index, surf->triCount);
			}
		}

		state->prim.device->SetFVF(NULL);
	}

	// *
	// world (bsp/terrain) drawing

	unsigned int R_ReadPrimDrawSurfInt(game::GfxReadCmdBuf* cmdBuf)
	{
		return *cmdBuf->primDrawSurfPos++;
	}

	const unsigned int* R_ReadPrimDrawSurfData(game::GfxReadCmdBuf* cmdBuf, unsigned int count)
	{
		const auto pos = cmdBuf->primDrawSurfPos;
		cmdBuf->primDrawSurfPos += count;

		return pos;
	}

	bool R_ReadBspPreTessDrawSurfs(game::GfxReadCmdBuf* cmdBuf, game::GfxBspPreTessDrawSurf** list, unsigned int* count, unsigned int* baseIndex)
	{
		*count = R_ReadPrimDrawSurfInt(cmdBuf);
		if (!*count)
		{
			return false;
		}

		*baseIndex = R_ReadPrimDrawSurfInt(cmdBuf);
		*list = (game::GfxBspPreTessDrawSurf*)R_ReadPrimDrawSurfData(cmdBuf, *count);

		return true;
	}

	void R_DrawPreTessTris(game::GfxCmdBufSourceState* src [[maybe_unused]], game::GfxCmdBufPrimState* state, game::srfTriangles_t* tris, unsigned int baseIndex, unsigned int triCount)
	{
		const auto dev = dx->device;

		game::GfxMatrix world_mtx = {};
		world_mtx.m[0][0] = 1.0f;
		world_mtx.m[1][1] = 1.0f;
		world_mtx.m[2][2] = 1.0f;
		world_mtx.m[3][3] = 1.0f;

		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&world_mtx.m[0]));

		// texture alpha + vertex alpha (decal blending)
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		dev->SetStreamSource(0, gfx_world_vertexbuffer, WORLD_VERTEX_STRIDE * tris->firstVertex, WORLD_VERTEX_STRIDE);
		state->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, tris->vertexCount, baseIndex, triCount);
	}

	/**
	 * @brief completely rewritten R_DrawBspDrawSurfsPreTess to render bsp surfaces (world) using the fixed-function pipeline
	 */
	void R_DrawBspDrawSurfsPreTess(const unsigned int* primDrawSurfPos, game::GfxCmdBufSourceState* src, game::GfxCmdBufState* state)
	{
		const auto dev = dx->device;

		// #
		// setup fixed-function

		dev->SetFVF(WORLD_VERTEX_FORMAT);

		// def. needed or the game will render the mesh using shaders
		dev->SetVertexShader(nullptr);
		dev->SetPixelShader(nullptr);

		// #
		// draw prims

		unsigned int base_index, count;
		game::GfxBspPreTessDrawSurf* list;
		game::GfxReadCmdBuf cmd_buf = { primDrawSurfPos };

		while (R_ReadBspPreTessDrawSurfs(&cmd_buf, &list, &count, &base_index))
		{
			game::srfTriangles_t* prev_tris = nullptr;
			auto tri_count = 0u;
			auto base_vertex = -1;

			for (auto index = 0u; index < count; ++index)
			{
				const auto surf_index = static_cast<unsigned>(list[index].baseSurfIndex);
				if (surf_index >= static_cast<unsigned>(game::sp::rgp->world->surfaceCount))
				{
					__debugbreak();
					//Com_Error(0, "R_DrawBspDrawSurfsPreTess :: surf_index >= static_cast<unsigned>(game::rgp->world->surfaceCount)");
				}

				const auto bsp_surf = &game::sp::rgp->world->dpvs.surfaces[surf_index];
				const auto tris = &bsp_surf->tris;

				if (base_vertex != bsp_surf->tris.firstVertex)
				{
					// never triggers?
					if (tri_count)
					{
						R_DrawPreTessTris(src, &state->prim, prev_tris, base_index, tri_count);
						base_index += 3 * tri_count;
						tri_count = 0;
					}

					prev_tris = tris;
					base_vertex = tris->firstVertex;
				}

				tri_count += list[index].totalTriCount;
			}

			R_DrawPreTessTris(src, &state->prim, prev_tris, base_index, tri_count);
		}

		// #
		// restore everything for following meshes rendered via shaders

		dev->SetFVF(NULL);
	}

	// *
	// unbatched

	bool R_ReadBspDrawSurfs(const unsigned int** primDrawSurfPos, const unsigned __int16** list, unsigned int* count)
	{
		*count = *(*primDrawSurfPos)++;
		if (!*count)
		{
			return false;
		}
		*list = (const unsigned __int16*)*primDrawSurfPos;
		*primDrawSurfPos += (*count + 1) >> 1;
		return true;
	}

	void R_DrawBspTris(game::GfxCmdBufPrimState* state, game::srfTriangles_t* tris, unsigned int baseIndex, unsigned int triCount)
	{
		const auto dev = game::sp::dx->device;

		game::GfxMatrix world_mtx = {};
		world_mtx.m[0][0] = 1.0f;
		world_mtx.m[1][1] = 1.0f;
		world_mtx.m[2][2] = 1.0f;
		world_mtx.m[3][3] = 1.0f;
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&world_mtx.m[0]));

		// texture alpha + vertex alpha (decal blending)
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		//dev->SetStreamSource(0, gfx_world_vertexbuffer, WORLD_VERTEX_STRIDE * tris->firstVertex, WORLD_VERTEX_STRIDE);
		state->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, tris->vertexCount, baseIndex, triCount);
	}

	void R_DrawBspDrawSurfs(game::GfxTrianglesDrawStream* drawStream, game::GfxCmdBufPrimState* state)
	{
		const auto dev = game::sp::dx->device;

		// #
		// setup fixed-function
		dev->SetFVF(WORLD_VERTEX_FORMAT);

		// def. needed or the game will render the mesh using shaders
		dev->SetVertexShader(nullptr);
		dev->SetPixelShader(nullptr);


		// #
		// draw prims

		const std::uint16_t* list;
		game::srfTriangles_t* prev_tris = nullptr;
		unsigned int base_index = 0u, count = 0u, tri_count = 0u;
		auto base_vertex = -1;

		bool early_out = false;

		while (R_ReadBspDrawSurfs(&drawStream->primDrawSurfPos, &list, &count))
		{
			for (auto index = 0u; index < count; ++index)
			{
				const auto bsp_surf = &game::sp::rgp->world->dpvs.surfaces[list[index]];
				//if (base_vertex != bsp_surf->tris.firstVertex || base_index + 3u * tri_count != static_cast<unsigned>(bsp_surf->tris.baseIndex))
				if (base_vertex != bsp_surf->tris.firstVertex || tri_count + base_index + 2 * tri_count != static_cast<unsigned>(bsp_surf->tris.baseIndex))
				{
					if (prev_tris)
					{
						//const auto base = R_SetIndexData(state, &game::sp::rgp->world->draw.indices[prev_tris->baseIndex], tri_count);
						const auto base = R_SetIndexData(state, &game::sp::get_g_world_draw()->indices[prev_tris->baseIndex], tri_count);
						if (base < 0)
						{
							early_out = true;
							break;
						}

						R_DrawBspTris(state, prev_tris, base, tri_count);
					}

					tri_count = 0;
					prev_tris = &bsp_surf->tris;
					base_index = bsp_surf->tris.baseIndex;

					if (base_vertex != bsp_surf->tris.firstVertex)
					{
						base_vertex = bsp_surf->tris.firstVertex;
						dev->SetStreamSource(0, gfx_world_vertexbuffer, WORLD_VERTEX_STRIDE * bsp_surf->tris.firstVertex, WORLD_VERTEX_STRIDE);
					}
				}

				tri_count += bsp_surf->tris.triCount;
			}

			if (early_out)
			{
				break;
			}
		}

		if (!early_out && prev_tris)
		{
			const auto base = R_SetIndexData(state, &game::sp::get_g_world_draw()->indices[prev_tris->baseIndex], tri_count);
			R_DrawBspTris(state, prev_tris, base, tri_count);
		}

		dev->SetFVF(NULL);
	}

	/**
	 * @brief completely rewritten R_TessBModel to render brushmodels using the fixed-function pipeline
	 * - most challenging issue yet
	 */
	std::uint32_t R_TessBModel(game::GfxDrawSurfListArgs* listArgs, [[maybe_unused]] void* x, [[maybe_unused]] void* y)
	{
		const auto source = listArgs->context.source;
		const auto prim = &listArgs->context.state->prim;

		// #
		// setup fixed-function

		prim->device->SetFVF(WORLD_VERTEX_FORMAT);

		// def. needed or the game will render the mesh using shaders
		prim->device->SetVertexShader(nullptr);
		prim->device->SetPixelShader(nullptr);

		// texture alpha + vertex alpha (decal blending)
		prim->device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		prim->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		prim->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);


		// #
		// draw prims

		const auto draw_surf_list = &listArgs->info->drawSurfs[listArgs->firstDrawSurfIndex];
		const auto draw_surf_count = listArgs->info->drawSurfCount - listArgs->firstDrawSurfIndex;

		const auto draw_surf_sub_mask = 0xFFFFFFFFFFFF;

		game::GfxDrawSurf draw_surf = {};
		draw_surf.fields = draw_surf_list->fields;

		game::GfxDrawSurf draw_surf_key = {};
		draw_surf_key.packed = draw_surf.packed & draw_surf_sub_mask;

		const std::uint64_t first_surf = draw_surf_key.packed;
		auto draw_surf_index = 0u;

		while (true)
		{
			const auto bsurf = reinterpret_cast<game::BModelSurface*>(&source->u.input.data->surfsBuffer[4u * draw_surf.fields.objectId]);

			// #
			// transform

			float model_axis[3][3] = {};

			const auto placement = bsurf->placement;
			utils::unit_quat_to_axis(placement->base.quat, model_axis);
			const auto scale = placement->scale;

			//const auto mtx = source->matrices.matrix[0].m;
			float mtx[4][4] = {};

			// inlined ikMatrixSet44
			mtx[0][0] = model_axis[0][0] * scale;
			mtx[0][1] = model_axis[0][1] * scale;
			mtx[0][2] = model_axis[0][2] * scale;
			mtx[0][3] = 0.0f;

			mtx[1][0] = model_axis[1][0] * scale;
			mtx[1][1] = model_axis[1][1] * scale;
			mtx[1][2] = model_axis[1][2] * scale;
			mtx[1][3] = 0.0f;

			mtx[2][0] = model_axis[2][0] * scale;
			mtx[2][1] = model_axis[2][1] * scale;
			mtx[2][2] = model_axis[2][2] * scale;
			mtx[2][3] = 0.0f;

			mtx[3][0] = placement->base.origin[0];
			mtx[3][1] = placement->base.origin[1];
			mtx[3][2] = placement->base.origin[2];
			mtx[3][3] = 1.0f;

			prim->device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

			// #
			// ------

			const auto gfxsurf = bsurf->surf;
			const auto base_vertex = WORLD_VERTEX_STRIDE * gfxsurf->tris.firstVertex;

			if (prim->streams[0].vb != gfx_world_vertexbuffer || prim->streams[0].offset != base_vertex || prim->streams[0].stride != WORLD_VERTEX_STRIDE)
			{
				prim->streams[0].vb = gfx_world_vertexbuffer;
				prim->streams[0].offset = base_vertex;
				prim->streams[0].stride = WORLD_VERTEX_STRIDE;
				prim->device->SetStreamSource(0, gfx_world_vertexbuffer, base_vertex, WORLD_VERTEX_STRIDE);
			}

			const auto base_index = R_SetIndexData(prim, &game::sp::get_g_world_draw()->indices[gfxsurf->tris.baseIndex], gfxsurf->tris.triCount);
			prim->device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, gfxsurf->tris.vertexCount, base_index, gfxsurf->tris.triCount);

			++draw_surf_index;
			if (draw_surf_index == draw_surf_count)
			{
				break;
			}

			draw_surf.fields = draw_surf_list[draw_surf_index].fields;
			draw_surf_key.packed = draw_surf_list[draw_surf_index].packed & draw_surf_sub_mask;

			if (draw_surf_key.packed != first_surf)
			{
				break;
			}
		}

		prim->device->SetFVF(NULL);
		return draw_surf_index;
	}



	// *
	// effects

	void R_TessCodeMeshList_begin(game::GfxDrawSurfListArgs* listArgs)
	{
		const auto source = listArgs->context.source;
		const auto prim = &listArgs->context.state->prim;

		// #
		// setup fixed-function

		prim->device->SetFVF(MODEL_VERTEX_FORMAT);

		// def. needed or the game will render the mesh using shaders
		prim->device->SetVertexShader(nullptr);
		prim->device->SetPixelShader(nullptr);

		// vertices are already in 'world space' so origin is at 0 0 0
		float mtx[4][4] = {};
		mtx[0][0] = 1.0f; mtx[0][1] = 0.0f; mtx[0][2] = 0.0f; mtx[0][3] = 0.0f;
		mtx[1][0] = 0.0f; mtx[1][1] = 1.0f; mtx[1][2] = 0.0f; mtx[1][3] = 0.0f;
		mtx[2][0] = 0.0f; mtx[2][1] = 0.0f; mtx[2][2] = 1.0f; mtx[2][3] = 0.0f;
		mtx[3][0] = 0.0f; mtx[3][1] = 0.0f; mtx[3][2] = 0.0f; mtx[3][3] = 1.0f;

		prim->device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&mtx));

#if !USE_NEW_FX_SYS // old way

		// #
		// unpack codemesh vertex data and place new data into the dynamic vertex buffer

		void* og_buffer_data;
		if (const auto hr = source->u.input.data->codeMeshPtr->vb.buffer->Lock(0, source->u.input.data->codeMeshPtr->vb.used, &og_buffer_data, D3DLOCK_READONLY);
			hr < 0)
		{
			//R_FatalLockError(hr);
			//game::sp::Com_Error(0, "Fatal lock error - codeMesh :: R_TessCodeMeshList_begin");
			__debugbreak();
		}

		if ((int)(source->u.input.data->codeMeshPtr->vb.used + game::sp::gfx_buf->dynamicVertexBuffer->used) > game::sp::gfx_buf->dynamicVertexBuffer->total)
		{
			game::sp::gfx_buf->dynamicVertexBuffer->used = 0;
		}

		// R_SetVertexData
		void* buffer_data;
		if (const auto hr = game::sp::gfx_buf->dynamicVertexBuffer->buffer->Lock(
			game::sp::gfx_buf->dynamicVertexBuffer->used, source->u.input.data->codeMeshPtr->vb.used, &buffer_data,
				game::sp::gfx_buf->dynamicVertexBuffer->used != 0 ? 0x1000 : 0x2000); 
					hr < 0)
		{
			//R_FatalLockError(hr);
			//game::sp::Com_Error(0, "Fatal lock error - dynamicVertexBuffer :: R_TessCodeMeshList_begin");
			__debugbreak();
		}

		if (!og_buffer_data || !buffer_data)
		{
			return;
		}

		// #
		// unpack verts

		for (auto i = 0u; i * source->u.input.data->codeMeshPtr->vertSize < (unsigned)source->u.input.data->codeMeshPtr->vb.used && i < 0x4000; i++)
		{
			// position of vert within the vertex buffer
			const auto v_pos_in_buffer = i * source->u.input.data->codeMeshPtr->vertSize; // size of GfxPackedVertex

			// interpret GfxPackedVertex as unpacked_model_vert
			const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

			// interpret GfxPackedVertex as GfxPackedVertex 
			const auto src_vert = reinterpret_cast<game::GfxPackedVertex*>(((DWORD)og_buffer_data + v_pos_in_buffer));

			// vert pos
			v->pos[0] = src_vert->xyz[0];
			v->pos[1] = src_vert->xyz[1];
			v->pos[2] = src_vert->xyz[2];

			// normal unpacking in a cod4 hlsl shader:
			// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
			// temp0.xyz = temp0.www * temp0;

			const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
			v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
			v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
			v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

			// uv's
			game::sp::Vec2UnpackTexCoords(src_vert->texCoord.packed, v->texcoord);
		}

		source->u.input.data->codeMeshPtr->vb.buffer->Unlock();
		game::sp::gfx_buf->dynamicVertexBuffer->buffer->Unlock();

		const std::uint32_t vert_offset = game::sp::gfx_buf->dynamicVertexBuffer->used;
		game::sp::gfx_buf->dynamicVertexBuffer->used += source->u.input.data->codeMeshPtr->vb.used;

		// #
		// #

		if (prim->streams[0].vb != game::sp::gfx_buf->dynamicVertexBuffer->buffer || prim->streams[0].offset != vert_offset || prim->streams[0].stride != MODEL_VERTEX_STRIDE)
		{
			prim->streams[0].vb = game::sp::gfx_buf->dynamicVertexBuffer->buffer;
			prim->streams[0].offset = vert_offset;
			prim->streams[0].stride = MODEL_VERTEX_STRIDE;
			prim->device->SetStreamSource(0, game::sp::gfx_buf->dynamicVertexBuffer->buffer, vert_offset, MODEL_VERTEX_STRIDE);
		}

#else
		if (prim->streams[0].vb != dynamic_codemesh_vb || prim->streams[0].offset != 0 || prim->streams[0].stride != MODEL_VERTEX_STRIDE)
		{
			prim->streams[0].vb = dynamic_codemesh_vb;
			prim->streams[0].offset = 0;
			prim->streams[0].stride = MODEL_VERTEX_STRIDE;
			prim->device->SetStreamSource(0, dynamic_codemesh_vb, 0, MODEL_VERTEX_STRIDE);
		}
#endif

		// R_TessCodeMeshList :: game code will now render all codemesh drawsurfs - functions nop'd:
		// > R_UpdateVertexDecl - sets vertexshader
		// > R_SetStreamSource  - sets vertexbuffer (codeMesh.vb)
	}

	// reset ff at the end of R_TessCodeMeshList
	void R_TessCodeMeshList_end()
	{
		const auto dev = game::sp::dx->device;
		dev->SetFVF(NULL);
	}

	__declspec(naked) void R_TessCodeMeshList_begin_stub()
	{
		const static uint32_t retn_addr = 0x73A639;
		__asm
		{
			pushad;
			push	eax; // listArgs
			call	R_TessCodeMeshList_begin;
			add		esp, 4;
			popad;

			// og code
			push    ebx;
			push    esi;
			mov     esi, [eax + 4];
			jmp		retn_addr;
		}
	}

	__declspec(naked) void R_TessCodeMeshList_end_stub()
	{
		const static uint32_t retn_addr = 0x73A903;
		__asm
		{
			pushad;
			call	R_TessCodeMeshList_end;
			popad;

			// og code
			pop     edi;
			pop     esi;
			pop     ebx;
			mov     esp, ebp;
			jmp		retn_addr;
		}
	}

	// *
	// build buffers

	void build_gfxworld_buffers()
	{
		const auto dev = dx->device;
		void* vertex_buffer_data = nullptr;

		if (rgp->world)
		{
			if (gfx_world_vertexbuffer)
			{
				gfx_world_vertexbuffer->Release();
				gfx_world_vertexbuffer = nullptr;

				__debugbreak();
				//Com_Error(0, "build_gfxworld_buffers :: gfx_world_vertexbuffer != nullptr");
			}

			if (auto hr = dev->CreateVertexBuffer(WORLD_VERTEX_STRIDE * rgp->world->draw.vertexCount, D3DUSAGE_WRITEONLY, WORLD_VERTEX_FORMAT, D3DPOOL_DEFAULT, &gfx_world_vertexbuffer, nullptr);
				hr >= 0)
			{
				if (hr = gfx_world_vertexbuffer->Lock(0, 0, &vertex_buffer_data, 0);
					hr >= 0)
				{
					/*	struct GfxWorldVertex = 44 bytes
					{
						float xyz[3];
						float binormalSign;
						GfxColor color;
						float texCoord[2];
						float lmapCoord[2];
						PackedUnitVec normal;
						PackedUnitVec tangent;
					};*/

					// unpack normal so we can use fixed-function rendering with normals
					for (auto i = 0u; i < rgp->world->draw.vertexCount; i++)
					{
						// packed source vertex
						const auto src_vert = rgp->world->draw.vd.vertices[i];

						// position of our unpacked vert within the vertex buffer
						const auto v_pos_in_buffer = i * WORLD_VERTEX_STRIDE; // pos-xyz ; normal-xyz ; texcoords uv = 32 byte 
						const auto v = reinterpret_cast<unpacked_world_vert*>(((DWORD)vertex_buffer_data + v_pos_in_buffer));

						// vert pos
						v->pos[0] = src_vert.xyz[0];
						v->pos[1] = src_vert.xyz[1];
						v->pos[2] = src_vert.xyz[2];

						// unpack and assign vert normal

						// normal unpacking in a cod4 hlsl shader:
						// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
						// temp0.xyz = temp0.www * temp0;

						const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
						v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
						v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
						v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert.normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

						// packed vertex color : rgb values are most likely used for 'decal' blending 
						// clear color so textures dont look likethey are covered in unicorn vomit
						auto col = src_vert.color;
						col.array[0] = 0xff;
						col.array[1] = 0xff;
						col.array[2] = 0xff;
						//col.array[3] = 0xff;

						// packed vertex color : used for alpha blending of decals
						v->color = col.packed;

						// uv's
						v->texcoord[0] = src_vert.texCoord[0];
						v->texcoord[1] = src_vert.texCoord[1];
					}

					gfx_world_vertexbuffer->Unlock();
				}
				else
				{
					gfx_world_vertexbuffer->Release();
					gfx_world_vertexbuffer = nullptr;
				}
			}
		}
	}

	// called on map load (cg_init)
	__declspec(naked) void init_fixed_function_buffers_stub()
	{
		const static uint32_t stock_func = 0x5C4BA0;
		const static uint32_t retn_addr = 0x64FEB2;
		__asm
		{
			pushad;
			call	build_gfxworld_buffers;
			call	main_module::on_map_load;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}

	// *
	// cleanup buffers

	void free_fixed_function_buffers()
	{
		// #
		// cleanup world buffer

		if (gfx_world_vertexbuffer)
		{
			gfx_world_vertexbuffer->Release();
			gfx_world_vertexbuffer = nullptr;
		}
	}

	// called on renderer shutdown (R_Shutdown)
	__declspec(naked) void free_fixed_function_buffers_stub()
	{
		const static uint32_t stock_func = 0x6ED220; // R_ResetModelLighting
		const static uint32_t retn_addr = 0x6B845B;
		__asm
		{
			pushad;
			call	free_fixed_function_buffers;
			call	main_module::on_map_shutdown;
			popad;

			call	stock_func;
			jmp		retn_addr;
		}
	}

//#define ZNEAR_TEST // znear doesnt really change anything in this game
#ifdef ZNEAR_TEST
	game::dvar_s r_znear = {};
	game::dvar_s* r_znear_ptr = nullptr;
	game::dvar_s r_znear_depthhack = {};
	game::dvar_s* r_znear_depthhack_ptr = nullptr;
#endif

	void pre_rb_draw3d()
	{
		//const auto dev = dx->device;
		//const auto source = game::sp::gfxCmdBufSourceState;

#ifdef ZNEAR_TEST
		if (const auto var = game::sp::Dvar_FindVar("r_sunflare_fadein"); var)
		{
			r_znear.current.value = var->current.value;
		}

		if (const auto var = game::sp::Dvar_FindVar("r_sunflare_fadeout"); var)
		{
			r_znear_depthhack.current.value = -var->current.value;
		}
#endif
		//rtx::setup_rtx(source->u.input.data->viewParms);

#if USE_NEW_FX_SYS

		// alloc buffer once
		if (static bool alloc_dynamic_codemesh_vertexbuffer = false; !alloc_dynamic_codemesh_vertexbuffer)
		{
			if (const auto hr = dev->CreateVertexBuffer(source->u.input.data->codeMeshPtr->vb.total, D3DUSAGE_WRITEONLY, MODEL_VERTEX_FORMAT, D3DPOOL_DEFAULT, &dynamic_codemesh_vb, nullptr);
				hr > 0)
			{
				__debugbreak();
			}

			alloc_dynamic_codemesh_vertexbuffer = true;
		}

		// #
		// lock codemesh vb -> unpack vertex data and place new data into the dynamic_codemesh_vb

		void* og_buffer_data;
		if (const auto hr = source->u.input.data->codeMeshPtr->vb.buffer->Lock(0, source->u.input.data->codeMeshPtr->vb.used, &og_buffer_data, D3DLOCK_READONLY);
			hr < 0)
		{
			//R_FatalLockError(hr);
			//game::sp::Com_Error(0, "Fatal lock error - codeMesh :: R_TessCodeMeshList_begin");
			__debugbreak();
		}

		// #

		auto i = 0u;

		void* buffer_data = nullptr;
		if (const auto hr = dynamic_codemesh_vb->Lock(0, source->u.input.data->codeMeshPtr->vb.used, &buffer_data, D3DUSAGE_WRITEONLY);
			hr >= 0)
		{
			for (i = 0u; i * source->u.input.data->codeMeshPtr->vertSize < (unsigned)source->u.input.data->codeMeshPtr->vb.used && i < 0x4000; i++)
			{
				// position of vert within the vertex buffer
				const auto v_pos_in_buffer = i * source->u.input.data->codeMeshPtr->vertSize; // size of GfxPackedVertex

				/*if (v_pos_in_buffer > source->data->codeMesh.vb.used)
				{
					break;
				}*/

				// interpret GfxPackedVertex as unpacked_model_vert
				const auto v = reinterpret_cast<unpacked_model_vert*>(((DWORD)buffer_data + v_pos_in_buffer));

				// interpret GfxPackedVertex as GfxPackedVertex 
				const auto src_vert = reinterpret_cast<game::GfxPackedVertex*>(((DWORD)og_buffer_data + v_pos_in_buffer));
				//const auto src_vert = reinterpret_cast<game::GfxPackedVertex*>(source->u.input.data->codeMeshPtr->vb.verts)[i];


				// vert pos
				v->pos[0] = src_vert->xyz[0];
				v->pos[1] = src_vert->xyz[1];
				v->pos[2] = src_vert->xyz[2];

				// normal unpacking in a cod4 hlsl shader:
				// temp0	 = i.normal * float4(0.007874016, 0.007874016, 0.007874016, 0.003921569) + float4(-1, -1, -1, 0.7529412);
				// temp0.xyz = temp0.www * temp0;

				const auto scale = static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[3])) * (1.0f / 255.0f) + 0.7529412f;
				v->normal[0] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[0])) * (1.0f / 127.0f) + -1.0f) * scale;
				v->normal[1] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[1])) * (1.0f / 127.0f) + -1.0f) * scale;
				v->normal[2] = (static_cast<float>(static_cast<std::uint8_t>(src_vert->normal.array[2])) * (1.0f / 127.0f) + -1.0f) * scale;

				// uv's
				game::sp::Vec2UnpackTexCoords(src_vert->texCoord.packed, v->texcoord);
			}

			dynamic_codemesh_vb->Unlock();
		}
		else
		{
			dynamic_codemesh_vb->Release();
			dynamic_codemesh_vb = nullptr;
		}

		const auto y = i;

		// unlock og codemesh vb
		//source->u.input.data->codeMeshPtr->vb.buffer->Unlock();

#endif

	}

	__declspec(naked) void rb_draw3d_internal_stub()
	{
		const static uint32_t retn_addr = 0x6D2ECC;
		const static uint32_t retn_jz = 0x6D2EED;
		__asm
		{
			pushad;
			call	pre_rb_draw3d;
			popad;

			// og instructions
			sub     ecx, 0;
			push    esi;
			jz      loc_6D2EED;
			jmp		retn_addr;

		loc_6D2EED:
			jmp		retn_jz;

		}
	}

	// *
	// *

#ifdef ZNEAR_TEST
	__declspec(naked) void add_znear_stub()
	{
		const static uint32_t retn_addr = 0x6C802E;
		__asm
		{
			mov		eax, r_znear_ptr;
			movss   xmm0, dword ptr[eax + 0x18]
			jmp		retn_addr;
		}
	}

	
	__declspec(naked) void add_znear_depthhack_stub()
	{
		const static uint32_t retn_addr = 0x6C8072;
		__asm
		{
			mov		eax, r_znear_depthhack_ptr;
			movss   xmm0, dword ptr[eax + 0x18]
			jmp		retn_addr;
		}
	}
#endif

	fixed_function::fixed_function()
	{
#ifdef ZNEAR_TEST
		// add znear val
		/*mov  eax, ?r_znear@@3PBUdvar_s@@B
		movss  xmm0, dword ptr [eax+18h]*/
		r_znear.current.value = 4.00195f;
		r_znear_ptr = &r_znear;
		utils::hook::nop(0x6C8026, 8);
		utils::hook(0x6C8026, add_znear_stub, HOOK_JUMP).install()->quick();


		r_znear_depthhack.current.value = 4.0f;
		r_znear_depthhack_ptr = &r_znear_depthhack;
		utils::hook::nop(0x6C806A, 8);
		utils::hook(0x6C806A, add_znear_depthhack_stub, HOOK_JUMP).install()->quick();
#endif

		// enable fullbright by default
		utils::hook::set<BYTE>(0x6CA3E0 + 1, 0x1);

		// fixed-function rendering of static models (R_TessStaticModelRigidDrawSurfList)
		utils::hook(0x7495B7, R_DrawStaticModelDrawSurfNonOptimized, HOOK_CALL).install()->quick();

		// #
		// fixed-function rendering of rigid xmodels 
		utils::hook(0x74AFC9, save_gfxmodel_for_R_DrawXModelRigidModelSurf, HOOK_JUMP).install()->quick(); // save GfxModelRigidSurface* for R_DrawXModelRigidModelSurf
		utils::hook(0x74B0B7, R_DrawXModelRigidModelSurf, HOOK_CALL).install()->quick(); // eg. viewmodel gun

		// #
		// fixed-function rendering of skinned (animated) models (R_TessXModelSkinnedDrawSurfList)
		utils::hook::nop(0x73C977, 6);
		utils::hook(0x73C977, R_DrawXModelSkinnedUncached_stub, HOOK_JUMP).install()->quick(); // eg. viewmodel hands

		// ^ rigid skinned (only very few models) - doesnt render?
		//utils::hook::nop(0x73DFAA, 6);
		//utils::hook(0x73DFAA, R_DrawXModelSkinnedUncached_stub2, HOOK_JUMP).install()->quick();

		// #
		// TODO fixed-function rendering of static skinned models
		utils::hook(0x749F6A, R_DrawStaticModelsSkinnedDrawSurf, HOOK_CALL).install()->quick();

		// #
		// fixed-function rendering of world surfaces (R_TessTrianglesPreTessList)
		utils::hook(0x73EC45, R_DrawBspDrawSurfsPreTess, HOOK_CALL).install()->quick(); // r_pretess 1
		utils::hook(0x73EA98, R_DrawBspDrawSurfs, HOOK_CALL).install()->quick(); // r_pretess 0 (without surface batching)

		// #
		// fixed-function rendering of brushmodels
		utils::hook(0x73EC60, R_TessBModel, HOOK_JUMP).install()->quick();


		// hook beginning of 'RB_Draw3DInternal' - not of use rn
		utils::hook::nop(0x6D2EC6, 6); utils::hook(0x6D2EC6, rb_draw3d_internal_stub, HOOK_JUMP).install()->quick();


		// fixed-function effects
		if (!flags::has_flag("stock_effects"))
		{
			//utils::hook::set<BYTE>(0x73D70F + 5, MAX_EFFECT_VERTS_FOR_HOOK); // change max verts from 0x4000 to 0x1000 
			utils::hook::nop(0x73A668, 5); // R_UpdateVertexDecl
			utils::hook::nop(0x73A759, 5); // R_SetStreamSource
			utils::hook(0x73A634, R_TessCodeMeshList_begin_stub, HOOK_JUMP).install()->quick();
			utils::hook(0x73A8FE, R_TessCodeMeshList_end_stub, HOOK_JUMP).install()->quick();
		}

		// ----

		// on map load :: build custom buffers for fixed-function rendering
		utils::hook(0x64FEAD, init_fixed_function_buffers_stub, HOOK_JUMP).install()->quick(); // CG_Init :: CG_NorthDirectionChanged call
		utils::hook(0x727D2A, XSurfaceOptimizeRigid_stub, HOOK_JUMP).install()->quick(); // stock XSurfaceOptimize :: XSurfaceOptimizeRigid call

		// on renderer shutdown :: release custom buffers used by fixed-function rendering
		utils::hook(0x6B8456, free_fixed_function_buffers_stub, HOOK_JUMP).install()->quick(); // R_Shutdown :: R_ResetModelLighting call
	}
}
