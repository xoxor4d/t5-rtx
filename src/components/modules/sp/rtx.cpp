#include "std_include.hpp"

#define HIDWORD(x)  (*((DWORD*)&(x)+1))

using namespace game::sp;

namespace components::sp
{
	void rtx::setup_rtx(game::GfxViewParms* view_parms)
	{
		const auto dev = dx->device;

		game::GfxMatrix world = {};
		if (view_parms->zFar != 0.0f)
		{
			world.m[3][0] = -view_parms->axis[2][2];
			world.m[3][1] = -view_parms->depthHackNearClip;
			world.m[3][2] = -view_parms->zNear;
		}

		// directly set matrices on the device so that rtx-remix finds the camera
		dev->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX*>(&world.m));
		dev->SetTransform(D3DTS_VIEW, reinterpret_cast<D3DMATRIX*>(&view_parms->viewMatrix.m));
		dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<D3DMATRIX*>(&view_parms->projectionMatrix.m));

		// needed for skysphere
		dev->SetRenderState(D3DRS_LIGHTING, FALSE);

		rtx::force_dvars_on_frame();

		if (!flags::has_flag("no_fog"))
		{
			const float fog_start = 1.0f;
			dev->SetRenderState(D3DRS_FOGENABLE, TRUE);
			dev->SetRenderState(D3DRS_FOGCOLOR, map_settings::m_color.packed);
			dev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
			dev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fog_start));
			dev->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&map_settings::m_max_distance));
		}

#if 0
		// update culling vars at the end of a frame (so we don't change culling behaviour mid-frame -> not safe)
		{
			// update world culling
			if (dvars::rtx_disable_world_culling)
			{
				rtx::loc_disable_world_culling = dvars::rtx_disable_world_culling->current.integer;
				rtx::loc_disable_world_culling = rtx::loc_disable_world_culling < 0 ? 0 :
					rtx::loc_disable_world_culling > 3 ? 3 : rtx::loc_disable_world_culling;
			}

			// update entity culling
			/*if (dvars::rtx_disable_entity_culling)
			{
				rtx::loc_disable_entity_culling = dvars::rtx_disable_entity_culling->current.enabled ? 1 : 0;
			}*/
		}
#endif
	}

	__declspec(naked) void r_renderscene_stub()
	{
		const static uint32_t retn_addr = 0x6C8DF1;
		__asm
		{
			pushad;
			push	ebx; // viewParms
			call	rtx::setup_rtx;
			add		esp, 4;
			popad;

			// og instructions
			mov     ecx, [esi + 0x183A8];
			jmp		retn_addr;
		}
	}

	// ------------------------

	void rtx::force_dvars_on_frame()
	{
		dvars::bool_override("r_smp_backend", false);
		dvars::bool_override("r_skinCache", false);
		dvars::bool_override("r_fastSkin", false);
		dvars::bool_override("r_smc_enable", false);
		dvars::bool_override("r_depthPrepass", false);
		dvars::bool_override("r_dof_enable", false);
		dvars::bool_override("r_distortion", false);

		if (dvars::r_showTess && dvars::r_showTess->current.enabled)
		{
			if (static bool enable_developer_once = false; !enable_developer_once)
			{
				if (const auto var = game::sp::Dvar_FindVar("developer");
					var && !var->current.enabled)
				{
					dvars::int_override("developer", 1);
				}

				enable_developer_once = true;
			}
		}
	}

	void rtx::set_dvar_defaults()
	{
#if 0
		dvars::rtx_disable_world_culling = game::sp::Dvar_RegisterEnum(
			/* name		*/ "rtx_disable_world_culling",
			/* enumData */ rtx::rtx_disable_world_culling_enum,
			/* default	*/ 1,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "Disable world culling. 'all' is needed for stable geometry hashes!\n"
			"- less: reduces culling to portals only (unstable world geo hashes!)\n"
			"- all: disable culling of all surfaces including models\n"
			"- all-but-models: disable culling of all surfaces excluding models");
#endif

		static const char* r_showTess_enum[] = { "off", "tech", "techset", "material", "vertexshader", "pixelshader" };
		dvars::r_showTess = game::sp::Dvar_RegisterEnum(
			/* name		*/ "r_showTess",
			/* enumData */ r_showTess_enum,
			/* default	*/ 0,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "surface data");

		dvars::r_showTessDist = game::sp::Dvar_RegisterFloat(
			/* name		*/ "r_showTessDist",
			/* default	*/ 1000.0f,
			/* min		*/ 0.0f,
			/* max		*/ 10000.0f,
			/* flags	*/ game::dvar_flags::none,
			/* desc		*/ "radius in which to draw r_showTess debug strings");

		dvars::rtx_water_uv_scale = game::sp::Dvar_RegisterFloat(
			/* name		*/ "rtx_water_uv_scale",
			/* default	*/ 0.01f,
			/* min		*/ 0.0001f,
			/* max		*/ 2.0f,
			/* flags	*/ game::dvar_flags::saved,
			/* desc		*/ "UV scale of water");

		if (!flags::has_flag("no_forced_lod"))
		{
			// force lod to LOD0
			if (const auto var = Dvar_FindVar("r_forceLod"); var)
			{
				var->current.integer = 0;
				var->flags = game::dvar_flags::userinfo;
			}
		}

		if (const auto var = Dvar_FindVar("r_lodScaleRigid"); var)
		{
			var->current.value = 1.0f;
			var->domain.value.min = 0.0f;
			var->domain.value.max = FLT_MAX;
			var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("r_lodScaleSkinned"); var)
		{
			var->domain.value.min = 0.0f;
			var->domain.value.max = FLT_MAX;
			var->flags = game::dvar_flags::userinfo;
		}

		// really slowing the game down with clouds rendered via shaders
		dvars::bool_override("fx_drawclouds", false);

		dvars::bool_override("r_pretess", true);
		dvars::bool_override("r_smp_worker", true);
		dvars::bool_override("sv_cheats", true);
	}

	__declspec(naked) void register_dvars_stub()
	{
		const static uint32_t stock_func = 0x70B210;
		const static uint32_t retn_addr = 0x6B833F;
		__asm
		{
			call	stock_func;

			pushad;
			call	rtx::set_dvar_defaults;
			popad;

			jmp		retn_addr;
		}
	}

	namespace cull
	{
		int _last_active_valid_cell = -1;

		void R_AddWorldSurfacesPortalWalk_hk(int camera_cell_index, game::DpvsView* dpvs)
		{
			const auto dpvsGlob = reinterpret_cast<game::DpvsGlob*>(0x3957100);

			// never show the complete map, only the last valid cell
			if (camera_cell_index < 0)
			{
				const auto cell = &game::sp::rgp->world->cells[_last_active_valid_cell];
				const auto cell_index = cell - game::sp::rgp->world->cells;
				game::sp::R_AddCellSurfacesAndCullGroupsInFrustumDelayed(cell, dpvs->frustumPlanes, dpvs->frustumPlaneCount, dpvs->frustumPlaneCount);
				dpvsGlob->cellVisibleBits[(cell_index >> 5) + 3] |= (1 << (cell_index & 0x1F)) & ~((1 << (cell_index & 0x1F)) & dpvsGlob->cellForceInvisibleBits[(cell_index >> 5) + 3]);
			}
			else
			{
				_last_active_valid_cell = camera_cell_index;

				// always add full cell the player is in (same as r_singlecell)
				const auto cell = &game::sp::rgp->world->cells[camera_cell_index];
				const auto cell_index = cell - game::sp::rgp->world->cells;


				/*dpvsGlob->views[0][0].frustumPlanes[0].coeffs[3] += 5000.0f;
				...
				dpvsGlob->views[0][0].frustumPlanes[4].coeffs[3] += 5000.0f;
				dpvsGlob->nearPlane.coeffs[2] += 0.5f;
				dpvsGlob->nearPlane.coeffs[3] -= 0.5f;*/

				// hack - disable most frustum culling
				dpvsGlob->views[0][0].frustumPlanes[0].coeffs[3] += 5000.0f;
				dpvsGlob->views[0][0].frustumPlanes[1].coeffs[3] += 5000.0f;
				dpvsGlob->views[0][0].frustumPlanes[2].coeffs[3] += 5000.0f;
				dpvsGlob->views[0][0].frustumPlanes[3].coeffs[3] += 5000.0f;
				dpvsGlob->views[0][0].frustumPlanes[4].coeffs[3] += 5000.0f;
				dpvsGlob->views[0][0].frustumPlanes[5].coeffs[3] += 5000.0f;
				dpvsGlob->nearPlane.coeffs[3] += 5000.0f; 

				game::sp::R_AddCellSurfacesAndCullGroupsInFrustumDelayed(cell, dpvs->frustumPlanes, dpvs->frustumPlaneCount, dpvs->frustumPlaneCount); // dpvs->frustumPlaneCount
				dpvsGlob->cellVisibleBits[(cell_index >> 5) + 3] |= (1 << (cell_index & 0x1F)) & ~((1 << (cell_index & 0x1F)) & dpvsGlob->cellForceInvisibleBits[(cell_index >> 5) + 3]);

				// R_VisitPortals
				utils::hook::call<void(__cdecl)(game::GfxCell*, game::DpvsPlane*, game::DpvsPlane*, int)>(0x6B3DA0)(cell, &dpvsGlob->nearPlane, dpvs->frustumPlanes, dpvs->frustumPlaneCount);
			}
		}

		__declspec(naked) void cell_stub()
		{
			const static uint32_t retn_addr = 0x6B6016;
			__asm
			{
				// ebx = world
				// esi = cameraCellIndex
				// eax = DpvsView

				push	eax;
				push	esi;
				call	R_AddWorldSurfacesPortalWalk_hk;
				add		esp, 8;
				jmp		retn_addr;
			}
		}

#if 0
		// R_AddWorldSurfacesPortalWalk
		__declspec(naked) void world_stub_01()
		{
			const static uint32_t retn_skip = 0x6B601C;
			const static uint32_t retn_stock = 0x6B5FB3;
			__asm
			{
				// jump if culling is less or disabled
				cmp		rtx::loc_disable_world_culling, 1;
				jge		SKIP;

				// og code
				mov		[esp + 0x10], eax;
				test    esi, esi
				jmp		retn_stock;

			SKIP:
				mov		[esp + 0x10], eax;
				jmp		retn_skip;
			}
		}

		int _skipped_cull = 0; // helper var
		__declspec(naked) void world_stub_02_reset_helper()
		{
			const static uint32_t retn_stock = 0x711BC2;
			__asm
			{
				mov		_skipped_cull, 0;
				mov     edx, esi;
				mov		[ecx + 0xC], ebx;
				jmp		retn_stock;
			}
		}

		__declspec(naked) void world_stub_02_skip_static_model()
		{
			const static uint32_t retn_stock = 0x72167A;
			const static uint32_t retn_stock_jumped = 0x72169D;
			__asm
			{
				// do we want to cull static models the way geo would be culled?
				cmp		rtx::loc_disable_world_culling, 3;
				jl		STOCK;

				// did we skip the culling check in 'r_cull_world_stub_02'?
				cmp		_skipped_cull, 1;
				je		SKIP;

			STOCK:		// og code
				test    ecx, ecx;
				jz		loc_72169D;
				mov     edx, [edi + 0x20];
				jmp		retn_stock;

			loc_72169D: // og code
				jmp		retn_stock_jumped;


			SKIP:		// skip static model rendering
				mov     edx, [edi + 0x20];
				jmp		retn_stock_jumped;
			}
		}

		// R_AddAabbTreeSurfacesInFrustum_r
		__declspec(naked) void world_stub_02()
		{
			const static uint32_t retn_skip = 0x7215DF;
			const static uint32_t retn_stock = 0x7215D9;
			__asm
			{
				// jump if culling is disabled
				cmp		rtx::loc_disable_world_culling, 2;
				jge		SKIP;

				// og code
				addss   xmm5, xmm6;
				comiss  xmm4, xmm5;
				jmp		retn_stock;

			SKIP: // jumped here because culling is disabled 
				addss   xmm5, xmm6;
				comiss  xmm4, xmm5;
				jnb		HACKED_CULLING;
				jmp		retn_skip;

			HACKED_CULLING: // jumped here because the game would have culled this object
				mov		_skipped_cull, 1;
				jmp		retn_skip;
			}
		}

		// R_AddAabbTreeSurfacesInFrustum_r
		__declspec(naked) void world_stub_03()
		{
			const static uint32_t retn_skip = 0x721645;
			const static uint32_t retn_stock = 0x721615;
			__asm
			{
				// jump if culling is less or disabled
				cmp		rtx::loc_disable_world_culling, 1;
				jge		SKIP;

				// og code
				addss   xmm5, xmm0;
				comiss  xmm5, xmm4;
				jmp		retn_stock;

			SKIP:
				addss   xmm5, xmm0;
				comiss  xmm5, xmm4;
				jmp		retn_skip;
			}
		}

		// R_AddCellSceneEntSurfacesInFrustumCmd
		__declspec(naked) void entities_stub()
		{
			const static uint32_t retn_skip = 0x74A31B;
			const static uint32_t retn_stock = 0x74A315;
			__asm
			{
				cmp		rtx::loc_disable_entity_culling, 1;
				je		SKIP;

				// stock op's
				and		[esp + 0xC], esi;
				mov     esi, [esp + 0x4C];
				jmp		retn_stock;

			SKIP:
				and		[esp + 0xC], esi;
				mov     esi, [esp + 0x4C];
				jmp		retn_skip;
			}
		}
#endif
	}

	rtx::rtx()
	{
		// *
		// general

		// hook beginning of 'R_RenderScene' to setup general stuff required for rtx-remix
		utils::hook::nop(0x6C8DEB, 6); utils::hook(0x6C8DEB, r_renderscene_stub, HOOK_JUMP).install()->quick();

		// stub after 'R_InitGraphicsApi' (NVAPI Entry) to re-register stock dvars
		utils::hook(0x6B833A, register_dvars_stub, HOOK_JUMP).install()->quick();

		// ------------------------------------------------------------------------

		// *
		// culling

		// rewrite some logic in 'R_AddWorldSurfacesPortalWalk'
		utils::hook::nop(0x6B5FC4, 9); utils::hook(0x6B5FC4, cull::cell_stub, HOOK_JUMP).install()->quick();

		// ^ - never show all cells at once when the camera cell index is < 0, we handle that in the func above
		utils::hook::nop(0x6B5FB3, 2);

		// the next 3 nops together force crash the game after drinking a perk bottle because of CEG checks (now handled in the CEG module)
		
		// disable (most) frustum culling
		utils::hook::nop(0x7217DC, 2);

		// disable mins culling 
		utils::hook::nop(0x7215D9, 6);

		// ^ for smodels - TODO - make dvar for this?
		utils::hook::nop(0x72173F, 2);

		// never cull brushmodels via dpvs
		utils::hook::nop(0x74784D, 2);
		utils::hook::set<BYTE>(0x74785A, 0xEB); // ^

		// ^ scene ents (curtain on theater) 
		utils::hook::nop(0x747618, 2);

#if 0
		{
			// R_AddWorldSurfacesPortalWalk :: less culling
			utils::hook::nop(0x6B5FAD, 6); utils::hook(0x6B5FAD, cull::world_stub_01, HOOK_JUMP).install()->quick();

			// TODO ^ not use this or make optional with new option in dvar (bad fps)

			{
				// note: using 'rtx_disable_world_culling' = 'less' results in unstable world geometry hashes (but stable static model hashes)
				// -> using 'all-but-models' leaves culling logic for static models at 'less' but does not cull world geometry

				// R_AddCellStaticSurfacesInFrustumCmd :: stub used to reset the skip model check from the stub below
				utils::hook(0x711BBD, cull::world_stub_02_reset_helper, HOOK_JUMP).install()->quick();
				// R_AddAabbTreeSurfacesInFrustum_r :: check if culling mode 'all-but-models' is active - check note above
				utils::hook(0x721675, cull::world_stub_02_skip_static_model, HOOK_JUMP).install()->quick();

				// R_AddAabbTreeSurfacesInFrustum_r :: mins check
				utils::hook::nop(0x7215D2, 7); utils::hook(0x7215D2, cull::world_stub_02, HOOK_JUMP).install()->quick();
			}

			// R_AddAabbTreeSurfacesInFrustum_r :: maxs check
			utils::hook::nop(0x72160E, 7); utils::hook(0x72160E, cull::world_stub_03, HOOK_JUMP).install()->quick();

			// TODO: 0x6B0B19
			// R_AddCellSceneEntSurfacesInFrustumCmd :: active ents like destructible cars / players
			//utils::hook::nop(0x74A30D, 8); utils::hook(0x74A30D, cull::entities_stub, HOOK_JUMP).install()->quick();

			// TODO:
			// R_AddCellSceneEntSurfacesInFrustumCmd :: brushmodels
			//utils::hook::nop(0x74A50A, 2);

			//dvars::rtx_disable_entity_culling = game::sp::Dvar_RegisterBool(
			//	/* name		*/ "rtx_disable_entity_culling",
			//	/* default	*/ true,
			//	/* flags	*/ game::dvar_flags::saved,
			//	/* desc		*/ "Disable culling of game entities (script objects/destructible cars ...)");
		}
#endif
	}
}
