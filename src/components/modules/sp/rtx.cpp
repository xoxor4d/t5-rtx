#include "std_include.hpp"

#define HIDWORD(x)  (*((DWORD*)&(x)+1))

using namespace game::sp;

namespace components::sp
{
	void setup_rtx(game::GfxViewParms* view_parms)
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

		if (!flags::has_flag("no_fog"))
		{
			const float fog_start = 1.0f;
			dev->SetRenderState(D3DRS_FOGENABLE, TRUE);
			dev->SetRenderState(D3DRS_FOGCOLOR, map_settings::m_color.packed);
			dev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
			dev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fog_start));
			dev->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&map_settings::m_max_distance));
		}
	}

	__declspec(naked) void r_renderscene_stub()
	{
		const static uint32_t retn_addr = 0x6C8DF1;
		__asm
		{
			pushad;
			push	ebx; // viewParms
			call	setup_rtx;
			add		esp, 4;
			popad;

			// og instructions
			mov     ecx, [esi + 0x183A8];
			jmp		retn_addr;
		}
	}

	// ------------------------

	void set_dvar_defaults()
	{
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

		if (const auto var = Dvar_FindVar("r_skinCache"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("r_fastSkin"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("r_distortion"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("r_depthprepass"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("r_smc_enable"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("r_pretess"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		if (const auto var = Dvar_FindVar("fx_marks"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}

		// TODO: r_smp_backend - blocks input - freezes game

		/*if (const auto var = Dvar_FindVar("r_smp_backend"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}*/

		/*if (const auto var = Dvar_FindVar("r_smp_worker"); var)
		{
			var->current.enabled = false; var->flags = game::dvar_flags::userinfo;
		}*/

		if (const auto var = Dvar_FindVar("sv_cheats"); var)
		{
			var->current.enabled = true; var->flags = game::dvar_flags::userinfo;
		}
	}

	__declspec(naked) void register_dvars_stub()
	{
		const static uint32_t stock_func = 0x70B210;
		const static uint32_t retn_addr = 0x6B833F;
		__asm
		{
			call	stock_func;

			pushad;
			call	set_dvar_defaults;
			popad;

			jmp		retn_addr;
		}
	}

	// *
	// load custom fastfile containing required custom assets

	//void load_common_fast_files()
	//{
	//	const char** zone_code_post_gfx = reinterpret_cast<const char**>(0x3BF6800);
	//	const char** zone_patch = reinterpret_cast<const char**>(0x3BF6804);
	//	const char** zone_ui = reinterpret_cast<const char**>(0x3BF6808);
	//	const char** zone_common = reinterpret_cast<const char**>(0x3BF680C);
	//	const char** zone_localized_common = reinterpret_cast<const char**>(0x3BF6814);
	//	const char** zone_mod = reinterpret_cast<const char**>(0x3BF6818);

	//	int i = 0;
	//	game::XZoneInfo xzone_info_stack[8];

	//	// ------------------------------------

	//	xzone_info_stack[i].name = *zone_code_post_gfx;
	//	xzone_info_stack[i].allocFlags = 4;
	//	xzone_info_stack[i].freeFlags = 0;
	//	++i;

	//	// ------------------------------------

	//	// unused in sp
	//	/*if (zone_localized_code_post_gfx)
	//	{
	//		xzone_info_stack[i].name = *game::zone_localized_code_post_gfx_mp;
	//		xzone_info_stack[i].allocFlags = game::XZONE_FLAGS::XZONE_LOC_POST_GFX;
	//		xzone_info_stack[i].freeFlags = game::XZONE_FLAGS::XZONE_LOC_POST_GFX_FREE;
	//		++i;
	//	}*/

	//	if (*zone_mod)
	//	{
	//		xzone_info_stack[i].name = *zone_mod;
	//		xzone_info_stack[i].allocFlags = 0x800;
	//		xzone_info_stack[i].freeFlags = 0;
	//		++i;

	//		game::sp::DB_LoadXAssets(&xzone_info_stack[0], i, 0);

	//		utils::hook::call<void(__cdecl)()>(0x6F6CE0)(); // R_BeginRemoteScreenUpdate
	//		utils::hook::call<void(__cdecl)()>(0x5A3320)(); // no clue
	//		utils::hook::call<void(__cdecl)()>(0x6F6D60)(); // R_EndRemoteScreenUpdate
	//		utils::hook::call<void(__cdecl)()>(0x5FDBF0)(); // no clue
	//		utils::hook::call<void(__cdecl)()>(0x48E560)(); // DB_SyncXAssets

	//		// start a new zone stack
	//		i = 0;
	//	}

	//	if (game::sp::DB_FileExists("xcommon_rtx", game::DB_FILE_EXISTS_PATH::DB_PATH_ZONE))
	//	{
	//		xzone_info_stack[i].name = "xcommon_rtx";
	//		xzone_info_stack[i].allocFlags = 4;
	//		xzone_info_stack[i].freeFlags = 0;
	//		++i;
	//	}

	//	// ---------------------------------------------------------------------------------------------------------

	//	if (*zone_ui) // not loaded when starting dedicated servers
	//	{
	//		xzone_info_stack[i].name = *zone_ui;
	//		xzone_info_stack[i].allocFlags = 16;
	//		xzone_info_stack[i].freeFlags = 0;
	//		++i;
	//	}

	//	if (*zone_localized_common) // not loaded on when starting dedicated servers
	//	{
	//		xzone_info_stack[i].name = *zone_localized_common;
	//		xzone_info_stack[i].allocFlags = 1;
	//		xzone_info_stack[i].freeFlags = 0;
	//		++i;
	//	}

	//	xzone_info_stack[i].name = *zone_common;
	//	xzone_info_stack[i].allocFlags = 8;
	//	xzone_info_stack[i].freeFlags = 0;
	//	++i;

	//	if (*zone_patch) // not loaded on when starting dedicated servers
	//	{
	//		xzone_info_stack[i].name = *zone_patch;
	//		xzone_info_stack[i].allocFlags = 512;
	//		xzone_info_stack[i].freeFlags = 0;
	//		++i;
	//	}

	//	// ------------------------------------

	//	game::sp::DB_LoadXAssets(&xzone_info_stack[0], i, 0);
	//}

	rtx::rtx()
	{
		// *
		// general

		// hook beginning of 'R_RenderScene' to setup general stuff required for rtx-remix
		utils::hook::nop(0x6C8DEB, 6); utils::hook(0x6C8DEB, r_renderscene_stub, HOOK_JUMP).install()->quick();

		// hook R_SetMaterial - material/technique replacing
		//utils::hook(0x741F1E, r_set_material, HOOK_CALL).install()->quick();

		// load custom fastfile containing required assets
		//utils::hook(0x6D63A5, load_common_fast_files, HOOK_CALL).install()->quick();

		// ------------------------------------------------------------------------

		// stub after 'R_InitGraphicsApi' (NVAPI Entry) to re-register stock dvars
		utils::hook(0x6B833A, register_dvars_stub, HOOK_JUMP).install()->quick();

		// ------------------------------------------------------------------------

		// disable dynent drawing (could have stable hashes but no stable lods right now)
		//utils::hook::nop(0x6DD7DD, 5);

		// note: dvar 'r_fovScaleThresholdRigid' can be used to stop fov related lod changes

		// implement r_forcelod logic for skinned models (R_SkinXModel)
		// TODO: this causes crashes on some maps
		//utils::hook::nop(0x6D9C10, 7);  utils::hook(0x6D9C10, skinned_xmodel_get_lod_for_dist_inlined, HOOK_JUMP).install()->quick();

		// implement r_forcelod logic for all other static models (R_AddAllStaticModelSurfacesCamera)
		//utils::hook::nop(0x732CD6, 7);  utils::hook(0x732CD6, rigid_xmodel_get_lod_for_dist_inlined, HOOK_JUMP).install()->quick();

		// r_warm_dpvs check @ 0x6E5D21 adds static models per cell
		// ^ @ 0x7398B4 frustum culling
		// DISABLE CULLING :: stop 'r_warm_dpvs' dvar from resetting itself je 0x74 -> jmp 0xEB
		//utils::hook::set<BYTE>(0x6DDED8, 0xEB);

		// do not add dynent bmodels
		//utils::hook::nop(0x6DD7BE, 5);

		// fix nullptr access (gfxsurface->material ptr) that can occur when culling is disabled (certain sp maps)
		//utils::hook::nop(0x6DACB0, 6); utils::hook(0x6DACB0, r_add_bmodel_surfaces_camera_stub, HOOK_JUMP).install()->quick();

		// hook FX_CullSphere to implement an additional radius check
		//utils::hook::nop(0x4B4120, 6); utils::hook(0x4B4120, fx_cullsphere_stub, HOOK_JUMP).install()->quick();

		/*dvars::fx_cull_elem_draw_radius = game::Dvar_RegisterFloat(
			"fx_cull_elem_draw_radius",
			1200.0f, 0.0f, 100000.0f,
			game::dvar_flags::archive,
			"fx elements inside this radius are not affected by culling (fx_cull_elem_draw)");*/

		// ------------------------------------------------------------------------

		/*command::add("rtx_sky_clear", [](command::params) { main_module::skysphere_spawn(0); });
		command::add("rtx_sky_desert", [](command::params) { main_module::skysphere_spawn(1); });
		command::add("rtx_sky_night", [](command::params) { main_module::skysphere_spawn(2); });
		command::add("rtx_sky_overcast", [](command::params) { main_module::skysphere_spawn(3); });
		command::add("rtx_sky_sunset", [](command::params) { main_module::skysphere_spawn(4); });*/
	}
}
