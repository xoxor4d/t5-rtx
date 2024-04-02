#include "std_include.hpp"
#define HIDWORD(x)  (*((DWORD*)&(x)+1))

// TODO:
// * changing hash on ui3d and cinematic (tv's) -> not unlit
// * port fx sphere culling from t4

using namespace game::sp;
namespace components::sp
{
	// rename window title to get rid of the trademark (R) which currently causes issues with the remix toolkit
	DWORD WINAPI main_module::find_window_loop(LPVOID)
	{
		const char* hwnd_title = "Call of Duty\xAE: BlackOps";
		std::uint32_t _time = 0;

		// wait for window creation
		while (!game::main_window)
		{
			// get main window hwnd
			game::main_window = FindWindowExA(nullptr, nullptr, "CoDBlackOps", hwnd_title);

			Sleep(100); _time += 100;
			if (_time >= 30000)
			{
				return TRUE;
			}
		}

		static auto version_str = std::string("t5-rtx");
		static bool version_str_init = false;

#ifndef GIT_DESCRIBE
#define GIT_DESCRIBE ""
#endif

		if (!version_str_init)
		{
			version_str += "-"s + GIT_DESCRIBE;
			version_str += game::is_game_mod ? " + game_mod" : "";
			version_str_init = true;
		}

		SetWindowTextA(game::main_window, version_str.c_str());
		return TRUE;
	}

	// *
	// fix resolution issues by removing duplicates returned by EnumAdapterModes
	// ^ this was fixed on the dxvk branch - TODO: remove when latest dxvk changes were merged into dxvk-remix

	namespace resolution
	{
		auto hash = [](const _D3DDISPLAYMODE& d) { return d.Width + 10 * d.Height + d.RefreshRate; };
		auto equal = [](const _D3DDISPLAYMODE& d1, const _D3DDISPLAYMODE& d2) { return d1.Width == d2.Width && d1.Height == d2.Height && d1.RefreshRate == d2.RefreshRate; };
		std::unordered_set<_D3DDISPLAYMODE, decltype(hash), decltype(equal)> modes(256, hash, equal);

		int enum_adapter_modes_intercept(std::uint32_t adapter_index, std::uint32_t mode_index)
		{
			_D3DDISPLAYMODE current = {};
			const auto hr = game::sp::dx->d3d9->EnumAdapterModes(adapter_index, D3DFMT_X8R8G8B8, mode_index, &current) < 0;
			modes.emplace(current);
			return hr;
		}

		__declspec(naked) void R_EnumDisplayModes_stub()
		{
			const static uint32_t retn_addr = 0x6B72A4;
			__asm
			{
				push	esi; // mode index
				push	ebx; // adapter index
				call	enum_adapter_modes_intercept;
				add		esp, 8;
				jmp		retn_addr;
			}
		}

		void enum_adapter_modes_write_array()
		{
			std::uint32_t idx = 0;
			for (auto& m : modes)
			{
				if (idx >= 256)
				{
					game::sp::Com_PrintMessage(0, "EnumAdapterModes : Failed to grab all possible resolutions. Array to small!\n", 0);
					break;
				}

				memcpy(&game::sp::dx->displayModes[idx], &m, sizeof(_D3DDISPLAYMODE));
				idx++;
			}
		}

		__declspec(naked) void R_EnumDisplayModes_stub2()
		{
			const static uint32_t R_CompareDisplayModes_addr = 0x6B7220;
			const static uint32_t retn_addr = 0x6B72DB;
			__asm
			{
				pushad;
				call	enum_adapter_modes_write_array;
				popad;

				push	R_CompareDisplayModes_addr;
				jmp		retn_addr;
			}
		}
	}

	void fix_aspect_ratio(game::GfxWindowParms* window_parms)
	{
		*reinterpret_cast<float*>(0x3966168) = (float)window_parms->displayWidth / (float)window_parms->displayHeight; //static_cast<float>(window_parms[7]) / static_cast<float>(window_parms[8]);
	}

	void __declspec(naked) fix_aspect_ratio_stub()
	{
		const static uint32_t retn_addr = 0x6B69BE;
		__asm
		{
			pushad;
			push	eax;
			call	fix_aspect_ratio;
			add		esp, 4;
			popad;

			xor		eax, eax;
			mov		eax, 1;

			jmp		retn_addr;
		}
	}

	void Field_Clear(game::field_t* edit)
	{
		((void(__cdecl*)(game::field_t*))0x50DB60)(edit);
	}

	bool Con_HasActiveAutoComplete()
	{
		// return conDrawInputGlob.matchIndex >= 0 && conDrawInputGlob.autoCompleteChoice[0];
		return (*(int*)0xC48A48 >= 0 && ((BYTE*)0xC48A08)[0]);
	}

	bool Con_CancelAutoComplete()
	{
		if (Con_HasActiveAutoComplete())
		{
			*(int*)0xC48A48 = -1;		// conDrawInputGlob.matchIndex
			((BYTE*)0xC48A08)[0] = 0;	// conDrawInputGlob.autoCompleteChoice[0]

			return true;
		}

		return false;
	}

	void Con_ToggleConsole()
	{
		const auto g_consoleField = reinterpret_cast<game::field_t*>(0xC72280);

		Field_Clear(g_consoleField);
		Con_CancelAutoComplete();

		g_consoleField->widthInPixels = 620;
		g_consoleField->charHeight = 16.0f;
		g_consoleField->fixedSize = true;

		*(bool*)0xC6924C = false;
		*(DWORD*)0x2910160 ^= 1;
	}

	// model:	[1] techset - [2] material
	// bsp:	[3] techset - [4] material
	// bmodel:	[5] techset - [6] material
	void main_module::RB_ShowTess(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, const float* center, const char* name, const float* color)
	{
		float offset_center[3];
		offset_center[0] = center[0];
		offset_center[1] = center[1];
		offset_center[2] = center[2];

		const auto dist_to_str = utils::distance(source->eyeOffset, offset_center);

		if (dvars::r_showTessDist)
		{
			if (dist_to_str > dvars::r_showTessDist->current.value && dvars::r_showTessDist->current.value != 0.0f)
			{
				return;
			}
		}

		bool viewmodel_string = false;
		auto font_scale = 0.25f;
		if (dist_to_str < 25.0f)
		{
			viewmodel_string = true;
			font_scale = 0.025f;
		}

		const game::MaterialTechnique* tech = nullptr;
		if (state->material && state->material->u_techset.localTechniqueSet->techniques[static_cast<std::uint8_t>(state->techType)])
		{
			tech = state->material->u_techset.localTechniqueSet->techniques[static_cast<std::uint8_t>(state->techType)];
		}

		if (const auto r_showTess = game::sp::Dvar_FindVar("r_showTess"); r_showTess && tech)
		{
			switch (r_showTess->current.integer)
			{
				case 1: // techset model
				case 3: // techset bsp
				case 5: // techset bmodel
				{
					// offset_center[2] = (((float)state->techType - 16.0f) * 0.3f) + offset_center[2];
					// header
					R_AddDebugString(&source->u.input.data->debugGlobals, offset_center, color, font_scale, utils::va("%s: %s", name, tech->name));
					font_scale *= 0.5f;

					offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;
					R_AddDebugString(&source->u.input.data->debugGlobals, offset_center, color, font_scale, utils::va("> [TQ]: %s", state->material->u_techset.localTechniqueSet->name));

					offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;
					R_AddDebugString(&source->u.input.data->debugGlobals, offset_center, color, font_scale, utils::va("> [VS] %s", tech->passArray[0].vertexShader ? tech->passArray[0].vertexShader->name : "<NONE>"));

					offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;
					R_AddDebugString(&source->u.input.data->debugGlobals, offset_center, color, font_scale, utils::va("> [PS] %s", tech->passArray[0].u_pixelshader.pixelShader ? tech->passArray[0].u_pixelshader.pixelShader->name : "<NONE>"));
					break;
				}

				case 2: // material model
				case 4: // material bsp
				case 6: // material bmodel
				{
					// header
					R_AddDebugString(&source->u.input.data->debugGlobals, offset_center, color, font_scale, utils::va("%s: %s", name, state->material->info.name));
					font_scale *= 0.5f;

					for (auto i = 0; i < state->material->textureCount; i++)
					{
						if (&state->material->textureTable[i] && state->material->textureTable[i].u.image && state->material->textureTable[i].u.image->name)
						{
							const auto img = state->material->textureTable[i].u.image;
							offset_center[2] -= viewmodel_string ? 0.25f : 2.5f;

							const char* semantic_str;
							switch(static_cast<std::uint8_t>(img->semantic))
							{
								case 0: semantic_str = "2D"; break;
								case 1: semantic_str = "F"; break;
								case 2: semantic_str = "C"; break;
								case 5: semantic_str = "N"; break;
								case 8: semantic_str = "S"; break;
								case 11: semantic_str = "W"; break;
								default: semantic_str = "C+"; break;
							}

							R_AddDebugString(&source->u.input.data->debugGlobals, offset_center, color, font_scale, utils::va("> [%s] %s", semantic_str, img->name)); // static_cast<std::uint8_t>(img->semantic)
						}
					}
					break;
				}

				default:
					break;
			}
		}
	}


	// TODO:
	// - call R_SkinXModelCmd (0x745280) directly - cmd placed @ 0x6CA0F2
	//game::SkinXModelCmd* test = nullptr;
	__declspec(naked) void add_skin_xmodel_cmd_stub()
	{
		const static uint32_t func_addr = 0x745280;
		const static uint32_t retn_addr = 0x6CA0FA;
		__asm
		{
			// hook replaced one push so we have to sub 4 from esp
			lea     edx, [esp + 0x74 - 4]; // SkinXModelCmd
			push    edx;
			//mov		test, edx;

			// hook replaced one push and we discard another so we have to sub 8 from esp
			mov[esp + 0x7C - 8], ebx; // all these write to SkinXModelCmd located in edx ^
			mov[esp + 0xA4 - 8], di;
			mov[esp + 0x80 - 8], ecx;

			call	func_addr;
			add		esp, 4;
			jmp		retn_addr;
		}
	}


	// - call R_AddAllSceneEntSurfacesCamera (usercall GfxViewInfo *a1@<eax>) (0x6B0B00) - cmd placed @ 0x6C7505
	//game::GfxViewInfo* view_test = nullptr;
	__declspec(naked) void add_sceneent_cmd_stub()
	{
		const static uint32_t func_addr = 0x6B0B00;
		const static uint32_t retn_addr = 0x6C750A;
		__asm
		{
			// hook replaced one push so we have to sub 4 from esp
			mov     eax, ebx; // GfxViewInfo
			//mov		view_test, eax;
			call	func_addr;
			jmp		retn_addr;
		}
	}

	// add_dobj_skin_cmd_stub
	__declspec(naked) void add_dobj_skin_cmd_stub()
	{
		const static uint32_t func_addr = 0x6CA150;
		const static uint32_t retn_addr = 0x6E2B64;
		__asm
		{
			// hook replaced one push so we have to sub 4 from esp
			lea     eax, [esp + 8 - 4];
			call	func_addr;
			jmp		retn_addr;
		}
	}

	__declspec(naked) void fx_draw_cmd_stub()
	{
		const static uint32_t func_addr = 0x60B390;
		const static uint32_t retn_addr = 0x6E2A5E;
		__asm
		{
			// nop'd one push so we have to sub 4 from esp
			// ecx is pushed
			call	func_addr;
			add     esp, 0x18 - 4;
			jmp		retn_addr;
		}
	}

	void force_renderer_dvars()
	{
		dvars::bool_override("r_smp_backend", false);
		dvars::bool_override("r_pretess", true);
		dvars::bool_override("r_skinCache", false);
		dvars::bool_override("r_fastSkin", false);
		dvars::bool_override("r_smc_enable", false);
		dvars::bool_override("r_depthPrepass", false);
		dvars::bool_override("r_dof_enable", false);
		dvars::bool_override("r_distortion", false);

		dvars::int_override("r_aaSamples", 1);

		dvars::bool_override("r_allow_intz", false);
		dvars::bool_override("r_allow_null_rt", false);

		dvars::bool_override("r_d3d9ex", false);
		dvars::bool_override("r_glow_allowed", false);
		dvars::bool_override("r_motionblur_enable", false);
		dvars::bool_override("r_specular", false);
		dvars::bool_override("r_vsync", false);
		dvars::bool_override("r_shaderWarming", false);
		dvars::bool_override("sm_enable", false);

		dvars::bool_override("r_enablePlayerShadow", true);
		dvars::bool_override("r_smp_worker", true);
		dvars::bool_override("r_multiGpu", false); // newest rtx (rr) crashes on init 

		// R_RegisterCmds
		utils::hook::call<void(__cdecl)()>(0x7244F0)();
	}

	int relocate_r_init()
	{
		utils::hook::call<void(__cdecl)()>(0x6B82E0)(); // R_Init

		CreateThread(nullptr, 0, main_module::find_window_loop, nullptr, 0, nullptr);

		return utils::hook::call<int(__cdecl)()>(0x49D640)(); // related to g_connectpaths
	}

#define RELOC_R_SHUTDOWN
#ifdef RELOC_R_SHUTDOWN
	void reloc_r_shutdown()
	{
		game::main_window = nullptr;

		// R_Shutdown
		utils::hook::call<void(__cdecl)()>(0x6B83A0)();
	}

	__declspec(naked) void vid_restart_to_complete_stub()
	{
		const static uint32_t vid_restart_complete_addr = 0x5D2F00;
		const static uint32_t retn_addr = 0x5B52DF;
		__asm
		{
			call	vid_restart_complete_addr;
			jmp		retn_addr;
		}
	}
#endif


	void rb_renderthread_stub()
	{
		// saved for later
		if (const auto var = game::sp::Dvar_FindVar("r_smp_backend"); var && !var->current.enabled)
		{
			// Sys_WaitRenderer
			utils::hook::call<void(__cdecl)()>(0x434750)();

			//while (!R_FinishedFrontendWorkerCmds())
			while (!utils::hook::call<BOOL(__cdecl)()>(0x4155F0)())
			{
				Sleep(1u);
			}

			// R_WaitFrontendWorkerCmds - needed ?!
			utils::hook::call<void(__cdecl)()>(0x658F70)();
		}


		// 5A06C0 - Sys_StopRenderer
		utils::hook::call<void(__cdecl)()>(0x5A06C0)();

		// 5A0720 - Sys_StartRenderer
		utils::hook::call<void(__cdecl)()>(0x5A0720)();
	}

	// call and move ptr after calling R_ToggleSmpFrame
	void fix_dynamic_buffers()
	{
		// og call - R_EndFrame
		utils::hook::call<void(__cdecl)()>(0x6D7B90)();

		// only when r_smp_backend is off - R_HandOffToBackend handles it otherwise
		if (const auto var = game::sp::Dvar_FindVar("r_smp_backend"); var && !var->current.enabled)
		{
			// RB_UpdateDynamicBuffers normally grabs the backenddata from smpData -> data_0
			// - place the backenddata ptr into data_0 manually 
			const auto smp_data = reinterpret_cast<game::GfxBackEndData**>(0x4643FD8);
			const auto frontend_data = game::sp::get_frontenddata_out();

			// safety check
			if (frontend_data->viewInfo && frontend_data->skinnedCacheVb)
			{
				*smp_data = frontend_data;

				// RB_UpdateDynamicBuffers
				utils::hook::call<void(__cdecl)()>(0x6EBBB0)();
				fixed_function::copy_fx_buffer();
			}
		}
	}

	void msg_loop()
	{
		tagMSG msg = {};
		while (PeekMessageA(&msg, nullptr, 0, 0, 0))
		{
			if (!GetMessageA(&msg, nullptr, 0, 0))
			{
				// Sys_Quit
				utils::hook::call<void(__cdecl)()>(0x5DF2B0)();
			}

			*(DWORD*)0x27706D4 = msg.time;

			TranslateMessage(&msg);
			DispatchMessageA(&msg);

#ifdef DEBUG_WINPROC
			printf("Msg loop: %d \n", msg.message);
#endif
		}
	}

	void __declspec(naked) sys_getevent_stub()
	{
		const static uint32_t retn_addr = 0x867C69;
		__asm
		{
			pushad;
			call	msg_loop;
			popad;

			jmp		retn_addr;
		}
	}

#if DEBUG
	//#define DEBUG_WINPROC
	#ifdef DEBUG_WINPROC
	void print_main_win_proc_msg(std::uint32_t msg)
	{
		printf("MSG: %d\n", msg);
	}

	void __declspec(naked) main_win_proc_stub()
	{
		const static uint32_t retn_addr = 0x554B43;
		__asm
		{
			pushad;
			push	edi; // msg
			call	print_main_win_proc_msg;
			add		esp, 4;
			popad;

			// og code
			push    ebx;
			mov     ebx, [esp + 0x50];
			jmp		retn_addr;
		}
	}
	#endif
#endif

	void fix_r_pretess()
	{
		game::sp::gfx_buf->preTessIndexBuffer->used = 0;
	}

	// fix_r_pretess_stub
	void __declspec(naked) fix_r_pretess_stub()
	{
		const static uint32_t retn_addr = 0x6C70B2;
		__asm
		{
			pushad;
			call	fix_r_pretess;
			popad;

			// og
			mov     edx, [eax + 0x16CBC8];
			jmp		retn_addr;
		}
	}

	int forcelod_get_lod(const int lod_count)
	{
		const auto& r_forceLod = game::sp::Dvar_FindVar("r_forceLod");

		if (r_forceLod->current.integer > lod_count // force lowest possible LOD
			|| (r_forceLod->current.integer >= lod_count)) // force second lowest possible LOD
		{
			return lod_count - 1 >= 0 ? lod_count - 1 : 0;
		}

		return r_forceLod->current.integer;
	}

	int forcelod_is_enabled()
	{
		const auto& r_forceLod = game::sp::Dvar_FindVar("r_forceLod");

		// 4 = none - disabled
		if (r_forceLod->current.integer == r_forceLod->reset.integer)
		{
			return 0;
		}

		return 1;
	}

	int xmodel_get_lod_for_dist_global_1 = 0;
	__declspec(naked) void XModelGetLodForDist_stub()
	{
		const static uint32_t break_addr = 0x523BF6;
		const static uint32_t og_logic_addr = 0x523BE0;
		__asm
		{
			pushad;
			push	ecx;					// save ecx
			call	forcelod_is_enabled;
			cmp		eax, 1;
			pop		ecx;					// restore ecx
			jne		OG_LOGIC;				// if r_forceLod != 1

			push	ecx;					// holds model->numLods
			call	forcelod_get_lod;
			add		esp, 4;
			mov		xmodel_get_lod_for_dist_global_1, eax;
			popad;

			mov		eax, xmodel_get_lod_for_dist_global_1; // move returned lodindex into the register the game expects it to be
			jmp		break_addr;


		OG_LOGIC:
			popad;
			lea     edx, [esi + 0x28];
			jmp		og_logic_addr;
		}
	}

	__declspec(naked) void reduce_playerheight_for_shadow_stub()
	{
		const static uint32_t retn_addr = 0x67F573;
		__asm
		{
			subss   xmm0, ds:main_module::m_reduce_playerheight_for_shadow;
			jmp		retn_addr;
		}
	}

	void ui_3d_render_to_texture(game::GfxViewInfo* view)
	{
		if (!view->isMissileCamera)
		{
			// R_UI3D_SetupTextureWindow (we need to scale Y * 2 because its rendered at 1024x512 and it doesnt auto fit the screen)
			utils::hook::call<void(__cdecl)(int window_index, float x, float y, float w, float h)>(0x6E21B0)(0, 0.0f, 0.0f, 1.0f, 2.0f);
			utils::hook::call<void(__cdecl)(int window_index, float x, float y, float w, float h)>(0x6E21B0)(1, 0.0f, 0.0f, 1.0f, 2.0f);

			// RB_UI3D_RenderToTexture
			utils::hook::call<void(__cdecl)(const void* cmds, game::GfxUI3DBackend* rbUI3D, game::GfxCmdBufInput* input)>(0x6E26A0)(view->cmds, &view->rbUI3D, &view->input);
		}

		const static uint32_t R_SetAndClearSceneTarget_addr = 0x6CFD20;
		__asm
		{
			pushad;
			mov		eax, view;
			call	R_SetAndClearSceneTarget_addr;
			popad;
		}

		if (view->isMissileCamera)
		{
			//R_InitLocalCmdBufState(&gfxCmdBufState); // done above
			game::sp::R_SetRenderTargetSize(game::sp::gfxCmdBufSourceState, 22);
			game::sp::R_SetRenderTarget(game::sp::gfxCmdBufSourceState, game::sp::gfxCmdBufState, game::R_RENDERTARGET_MISSILE_CAM);
		}
	}

	void __declspec(naked) RB_UI3D_RenderToTexture_stub()
	{
		const static uint32_t stock_addr = 0x6CFD20;
		const static uint32_t retn_addr = 0x6D0408;
		__asm
		{
			pushad;
			push	eax; // view
			call	ui_3d_render_to_texture;
			pop		eax;
			popad;

			// og code
			//call	stock_addr; // called in func above
			jmp		retn_addr;
		}
	}

	void __declspec(naked) RB_FullbrightDrawCommands_stub()
	{
		const static uint32_t func_addr = 0x6CFB30; // RB_EndSceneRendering
		const static uint32_t retn_addr = 0x6D051E;
		const static uint32_t skip_addr = 0x6D0521;
		__asm
		{
			xor		eax, eax;
			mov		al, byte ptr [edi + 0x38C4]; // isMissileCamera
			test	al, al;
			jnz		SKIP;

			push    ebx;
			push    esi;
			push    ebp;
			call	func_addr;
			jmp		retn_addr;

		SKIP:
			jmp		skip_addr;
		}
	}

	// ------------------------

	bool is_valid_technique_for_type(const game::Material* mat, const game::MaterialTechniqueType type)
	{
		if (mat
			&& mat->u_techset.techniqueSet
			&& mat->u_techset.techniqueSet->techniques
			&& mat->u_techset.techniqueSet->techniques[type])
		{
			return true;
		}

		return false;
	}

	bool switch_material(game::switch_material_t* swm, const char* material_name)
	{
		if (const auto	material = Material_RegisterHandle(material_name);
			material && material->textureTable && material->textureTable->u.image && material->textureTable->u.image->name
			&& !std::string_view(material->textureTable->u.image->name)._Equal("default"))
		{
			swm->material = material;
			swm->technique = nullptr;

			if (is_valid_technique_for_type(material, swm->technique_type))
			{
				swm->technique = material->u_techset.techniqueSet->techniques[swm->technique_type];
			}

			swm->switch_material = true;
			return true;
		}

		// return stock material if the above failed
		swm->material = swm->current_material;
		return false;
	}

	void switch_technique(game::switch_material_t* swm, game::Material* material)
	{
		if (material)
		{
			swm->technique = nullptr;

			if (is_valid_technique_for_type(material, swm->technique_type))
			{
				swm->technique = material->u_techset.techniqueSet->techniques[swm->technique_type];
			}

			swm->switch_technique = true;
			return;
		}

		// return stock technique if the above failed
		swm->technique = swm->current_technique;
	}

	int r_set_material(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, game::GfxDrawSurf drawSurf, const game::MaterialTechniqueType type)
	{
		if (!state->material)
		{
			return 0;
		}

		game::switch_material_t mat = {};

		mat.current_material = game::sp::rgp->sortedMaterials[(drawSurf.packed >> 31) & 4095];
		mat.current_technique = mat.current_material->u_techset.localTechniqueSet->techniques[type];

		mat.material = mat.current_material;
		mat.technique = mat.current_technique;
		mat.technique_type = type;

		if (state->material->info.sortKey == 5)
		{
			// maptype 5 = cube
			if (state->material->textureTable->u.image->mapType == 5)
			{
				mat.technique_type = game::TECHNIQUE_UNLIT;
				const auto sky_index = map_settings::settings()->sky_index;
				if (!sky_index)
				{
					switch_material(&mat, "mc/mtl_skybox_sp_kowloon");
				}
				else
				{
					switch_material(&mat, utils::va("mtl_rtx_sky%02d", sky_index));
				}
			}
		}

		// do not render shader based eye layer (sw4 texture) 
		if (   mat.current_material->info.gameFlags == 16u
			&& mat.current_material->textureCount == 3
			&& static_cast<std::uint8_t>(mat.current_material->info.sortKey) == 43u
			&& static_cast<std::uint8_t>(mat.current_material->stateFlags) == 17u)
		{
			if (std::string_view(mat.current_material->info.name).contains("mc/mtl_gen_eye_"))
			{
				return 0;
			}
		}

		if (!mat.switch_material && !mat.switch_technique && !mat.switch_technique_type)
		{
			if (state->origMaterial)
			{
				state->material = state->origMaterial;
			}
			if (state->origTechType)
			{
				state->techType = state->origTechType;
			}
		}

		// save the original material
		state->origMaterial = state->material;

		// only switch to a different technique_type
		if (mat.switch_technique_type)
		{
			switch_technique(&mat, mat.current_material);
		}

		state->material = mat.material;
		state->technique = mat.technique;

		if (!state->technique)
		{
			return 0;
		}

		if (!mat.switch_material && !mat.switch_technique && !mat.switch_technique_type)
		{
			if ((mat.technique_type == game::TECHNIQUE_EMISSIVE || mat.technique_type == game::TECHNIQUE_UNLIT) && (state->technique->flags & 0x10) != 0 && !*(WORD*)&source->pad[120])
			{
				return 0;
			}
		}

		if (((DWORD)drawSurf.fields.objectId & 0x780000) == 0 && (mat.current_material->info.gameFlags & 8) != 0)
		{
			return 0; // do not render the bsp skybox
		}

		state->origTechType = state->techType;
		state->techType = mat.technique_type;

		return 1;
	}

	// not in use
	void r_setup_pass(game::GfxCmdBufState* state)
	{
		for (auto i = 0; i < state->material->textureCount; i++)
		{
			if (state->material->textureTable[i].semantic == 11)
			{
				__debugbreak();
			}
		}
	}

	// not in use
	__declspec(naked) void r_setup_pass_stub()
	{
		const static uint32_t retn_addr = 0x7279E5;
		__asm
		{
			pushad;
			mov     eax, [esp + 0xC - 4 + 32];
			push	eax;
			call	r_setup_pass;
			add		esp, 4;
			popad;

			// og code
			push    ecx;
			mov     eax, [esp + 0xC];
			jmp		retn_addr;
		}
	}

	// render water surfaces
	void r_drawlit_post_resolve(game::GfxViewInfo* view)
	{
		const static uint32_t r_drawlit_addr = 0x737ED0;
		__asm
		{
			pushad;
			mov		esi, view;
			push	1;
			call	r_drawlit_addr;
			add		 esp, 4;
			popad;
		}
	}

	// call R_DrawLit with litphase POST_RESOLVE to render water surfaces
	// called inbetween 'R_DrawFullbrightLitCallback' and 'R_DrawFullbrightDecalCallback'
	__declspec(naked) void impl_r_drawlit_post_stub()
	{
		const static uint32_t retn_addr = 0x6D042A;
		__asm
		{
			pushad;
			push	edi;
			call	r_drawlit_post_resolve;
			add		esp, 4;
			popad;

			// og code
			lea     edx, [edi + 0x2320];
			jmp		retn_addr;
		}
	}


	int skip_image_load(game::GfxImage* img)
	{
		// 0x2 = color, 0x5 = normal, 0x8 = spec
		if (img->semantic == 0x5 || img->semantic == 0x8)
		{
			return 1;
		}

		return 0;
	}

	__declspec(naked) void load_image_stub()
	{
		const static uint32_t skip_img_addr = 0x736E2E;
		const static uint32_t og_logic_addr = 0x736E23;
		__asm
		{
			pushad;
			push	esi;					// img
			call	skip_image_load;
			pop		esi;
			cmp		eax, 1;
			jne		OG_LOGIC;

			popad;
			mov		[esi + 0x1C], eax;
			add     edi, 3;
			jmp		skip_img_addr;

			// og code
		OG_LOGIC:
			popad;
			push    esi;
			mov     edx, ebx;
			lea     ecx, [esp + 0x14];
			mov		[esi + 0x1C], eax;
			jmp		og_logic_addr;
		}
	}

	int check_stream_img(game::GfxImage* img)
	{
		if (!img->texture.basemap)
		{
			return 1;
		}

		return 0;
	}

	__declspec(naked) void load_image_stream_stub()
	{
		const static uint32_t skip_img_addr = 0x6F9170; //0x6F90A5;
		const static uint32_t retn_addr = 0x6F9069;

		const static uint32_t virt_free_func = 0x4F5F30;
		__asm
		{
			pushad;
			push	ebx;
			call	check_stream_img;
			add		esp, 4;
			cmp		eax, 1;
			jne		OG_LOGIC;

			popad;
			mov     eax, [ebx];
			jmp		skip_img_addr;

		OG_LOGIC:
			popad;
			mov     eax, [ebx];
			mov     ecx, [ebp - 0x10];

			/*push    ecx;
			call	virt_free_func;
			add		esp, 4;*/

			jmp		retn_addr;
		}
	}

	int check_sampler_img(game::GfxImage* img)
	{
		if (!img->texture.basemap)
		{
			/*if (img->name) game::sp::Com_PrintMessage(0, utils::va("sampler skip ... '%s'\n", img->name), 0);
			else game::sp::Com_PrintMessage(0, utils::va("sampler skip ... no name\n"), 0);*/
			return 1;
		}

		//game::sp::Com_PrintMessage(0, utils::va("sampler tex '%s'\n", img->name), 0);
		return 0;
	}

	__declspec(naked) void r_setsampler_stub()
	{
		const static uint32_t skip_addr = 0x725C4B;
		const static uint32_t retn_addr = 0x725BC9;
		__asm
		{
			pushad;
			push	ecx;
			call	check_sampler_img;
			add		esp, 4;
			cmp		eax, 1;
			jne		OG_LOGIC;

			popad;
			jmp		skip_addr;

		OG_LOGIC:
			popad;
			// og
			push    ebx;
			mov     bl, [esp + 0x10];
			jmp		retn_addr;
		}
	}

	void load_common_fast_files()
	{
		auto i = 0u;
		game::XZoneInfo info[2];

		//DB_ResetZoneSize(0);
		utils::hook::call<void(__cdecl)(int)>(0x621530)(0);

		if (const auto var = game::sp::Dvar_FindVar("useFastFile"); 
			var && var->current.enabled)
		{
			//DB_ReleaseXAssets();
			utils::hook::call<void(__cdecl)()>(0x62C260)();
		}

		if (const auto var = game::sp::Dvar_FindVar("zombietron");
			var && var->current.enabled)
		{
			info[0].name = nullptr;
			info[0].allocFlags = 0;
			info[0].freeFlags = 256;

			i++;

			if (game::sp::DB_FileExists("xcommon_rtx", game::DB_PATH_ZONE))
			{
				info[i].name = "xcommon_rtx";
				info[i].allocFlags = 0x4000000; // patch_override flag
				info[i].freeFlags = 0;
				++i;
			}

			//DB_LoadXAssets(info, 1u, 0);
			utils::hook::call<void(__cdecl)(game::XZoneInfo*, std::uint32_t, int)>(0x631B10)(info, i, 0);
			return;
		}

		if (const auto var = game::sp::Dvar_FindVar("zombiemode");
			var && var->current.enabled)
		{
			//if (DB_IsZoneLoaded((unsigned __int8*)"common_zombie"))
			if (utils::hook::call<bool(__cdecl)(const char*)>(0x528A20)("common_zombie"))
			{
				return;
			}
			info[0].name = "common_zombie";
		}
		else
		{
			//if (DB_IsZoneLoaded((unsigned __int8*)"common"))
			if (utils::hook::call<bool(__cdecl)(const char*)>(0x528A20)("common"))
			{
				return;
			}
			info[0].name = "common";
		}

		info[0].allocFlags = 256;
		info[0].freeFlags = 0;
		++i;

		if (!utils::hook::call<bool(__cdecl)(const char*)>(0x528A20)("xcommon_rtx") // DB_IsZoneLoaded
			&& game::sp::DB_FileExists("xcommon_rtx", game::DB_PATH_ZONE))
		{
			info[i].name = "xcommon_rtx";
			info[i].allocFlags = 0x4000000; // patch_override flag
			info[i].freeFlags = 0;
			++i;
		}

		//DB_ResetZoneSize(0);
		utils::hook::call<void(__cdecl)(int)>(0x621530)(0);

		//DB_LoadXAssets(info, 1u, 0);
		utils::hook::call<void(__cdecl)(game::XZoneInfo*, std::uint32_t, int)>(0x631B10)(info, i, 0);
	}

	void load_level_fastfile_hk(const char* map_name)
	{
		// Com_LoadLevelFastFiles - would normally call 'Com_LoadCommonFastFile' ^ hooked and loads xcommon_rtx
		// Com_LoadLevelFastFiles is entirely detoured in game_mod so we have to load our fastfile afterwards
		//utils::hook::call<void(__cdecl)(const char*)>(0x4C8890)(map_name);

		//if (game::is_game_mod)
		{
			if (!utils::hook::call<bool(__cdecl)(const char*)>(0x528A20)("xcommon_rtx") // DB_IsZoneLoaded
				&& game::sp::DB_FileExists("xcommon_rtx", game::DB_PATH_ZONE))
			{
				game::XZoneInfo info;
				info.name = "xcommon_rtx";
				info.allocFlags = 0x40; // patch_override flag 0x4000000 = world unloading issue
				info.freeFlags = 0;

				//DB_ResetZoneSize(0);
				utils::hook::call<void(__cdecl)(int)>(0x621530)(0);

				//DB_LoadXAssets(info, 1u, 0);
				utils::hook::call<void(__cdecl)(game::XZoneInfo*, std::uint32_t, int)>(0x631B10)(&info, 1, 0);
			}
		}

		utils::hook::call<void(__cdecl)(const char*)>(0x4C8890)(map_name);
	}

	// *
	// Event stubs

	// > fixed_function::init_fixed_function_buffers_stub
	void main_module::on_map_load()
	{
		map_settings::get()->set_settings_for_loaded_map();
		rtx::set_dvar_defaults();
	}

	// > fixed_function::free_fixed_function_buffers_stub
	void main_module::on_map_shutdown()
	{
		if (main_module::m_sky_texture)
		{
			main_module::m_sky_texture->Release();
			main_module::m_sky_texture = nullptr;
		}

		fixed_function::fixed_function::last_valid_sky_texture = nullptr;

		if (fixed_function::dynamic_codemesh_vb)
		{
			fixed_function::dynamic_codemesh_vb->Release();
			fixed_function::dynamic_codemesh_vb = nullptr;
		}
	}

	//void RB_Draw3D_stub()
	//{
	//	// RB_DrawComposites
	//	utils::hook::call<void(__cdecl)()>(0x740C30)();

	//	// RB_Draw3D
	//	utils::hook::call<void(__cdecl)()>(0x6EB760)();
	//}

	void cg_player_stub(game::DObj* obj, void* pose, unsigned int entnum, unsigned int renderFxFlags, float* lightingOrigin, float materialTime, float materialTime2, float burnFraction, float wetness, char altXModel, int textureOverrideIndex, void* dobjConstantSet, void* dobjConstantSetExtraCam, int lightingOriginToleranceSq, float scale)
	{
		int tex_override = -1;

		if (const auto var = game::sp::Dvar_FindVar("zombietron"); var && !var->current.enabled)
		{
			if (const auto	material = Material_RegisterHandle("mtl_rtx_playershadow"); material
				&& material->textureTable
				&& material->textureTable->u.image
				&& material->textureTable->u.image->name
				&& !std::string_view(material->textureTable->u.image->name)._Equal("default"))
			{
				int model_hash = 1337;
				for (auto i = 0u; i < static_cast<std::uint8_t>(obj->numModels); i++)
				{
					//int R_AllocTextureOverride(Material *material, unsigned __int16 modelIndexMask, GfxImage *img1, GfxImage *img2, int prevOverride)
					tex_override = utils::hook::call<int(__cdecl)(game::Material*, std::uint16_t, game::GfxImage*, game::GfxImage*, int)>(0x6BFBD0)
						(material, model_hash, material->textureTable->u.image, material->textureTable[1].u.image, i == 0 ? -1 : tex_override);
					model_hash += 2;
				}

				//auto x = game::sp::get_frontenddata_out();
				//obj->hidePartBits[3] = 0x0002ffff;
				
				// G_SpawnItem for hidepartbits logic
				//game::sp::g_entities->attachModelNames[0];
				//game::sp::G_EntDetachAll(game::sp::g_entities);
			}
		}

		// R_AddDObjToScene
		utils::hook::call<void(__cdecl)(game::DObj*, void*, unsigned int, unsigned int, float*, float, float, float, float, char, int, void*, void*, int, float)>(0x6BFDF0)
			(obj, pose, entnum, renderFxFlags, lightingOrigin, materialTime, materialTime2, burnFraction, wetness, altXModel, tex_override, dobjConstantSet, dobjConstantSetExtraCam, lightingOriginToleranceSq, scale);
	}

	main_module::main_module()
	{
		// rb_fullbrightdrawcommands :: call RB_UI3D_RenderToTexture to update the main menu 3d hud (tv)
		// texture (codeimage) is set via SetTexture in fixed_function::R_DrawXModelRigidModelSurf
		utils::hook(0x6D0403, RB_UI3D_RenderToTexture_stub, HOOK_JUMP).install()->quick();

		// extracam tests - do not call RB_EndSceneRendering when 'viewInfo->isMissileCamera'
		utils::hook::nop(0x6D0516, 8);
		utils::hook(0x6D0516, RB_FullbrightDrawCommands_stub, HOOK_JUMP).install()->quick();

		// disable RB_DrawSun
		utils::hook::nop(0x6D04FE, 5);

		// hook R_SetMaterial - material/technique replacing
		utils::hook(0x73F8C0, r_set_material, HOOK_JUMP).install()->quick();

		// only for testing
		//utils::hook(0x7279E0, r_setup_pass_stub, HOOK_JUMP).install()->quick();
		//utils::hook::nop(0x6C7164, 6); // test: add lit surfs

		// call R_DrawLit with litphase POST_RESOLVE to render water surfaces when in fullbright mode
		utils::hook::nop(0x6D0424, 6);
		utils::hook(0x6D0424, impl_r_drawlit_post_stub, HOOK_JUMP).install()->quick();


		// most of the following was done to make the game work with 'r_smp_backend' being disabled
		// disabling 'r_smp_backend' causes the game to not receive and handle mb/kb input .. and f's up a whole lot more
		// the dvar is required to fix gun jitter

		// set required dvars
		utils::hook(0x6B8300, force_renderer_dvars, HOOK_CALL).install()->quick();


		// relocate R_Init from RB_RenderThread to CL_InitRenderer (call r_init from the main thread)
		// - r_init, which creates the game window, is now called from the main thread instead of the renderer thread
		// - required so the message loop, which will also get relocated to the main thread further down below, receives the window messages
		utils::hook::nop(0x6EBEC9, 5); // do not call r_init from the renderer thread
		utils::hook(0x52F28A, relocate_r_init, HOOK_CALL).install()->quick(); // TODO: reimpl. R_Init in various other functions

#ifdef RELOC_R_SHUTDOWN
		// reimplement 'R_Shutdown' in 'CL_ShutdownRenderer'
		utils::hook(0x6B8559, reloc_r_shutdown, HOOK_CALL).install()->quick();
		utils::hook::nop(0x6B855E, 7); // ^ skip for loop in hooked func
		utils::hook::set<BYTE>(0x6B8565, 0xEB); // ^ skip for loop in hooked func

		// never use seamless resize logic in vid_restart_complete
		utils::hook::set<BYTE>(0x5D2F2A, 0xEB);

		// allow vid_restart_complete on listen server
		utils::hook::set<BYTE>(0x5D2F59, 0xEB);

		// call vid_restart_complete when vid_restart is called
		utils::hook(0x5B52A0, vid_restart_to_complete_stub, HOOK_CALL).install()->quick(); 

		// crashing func when quit calls 'R_Shutdown'
		utils::hook::nop(0x6B8518, 5);
		utils::hook::nop(0x6B71EC, 5);

		// R_ShutdownDirect3DInternal:: do not release dx.d3d9 (remix)
		utils::hook::set<BYTE>(0x6B8189, 0xEB);

		// change Error: 'Weapon index mismatch for ..' from DROP to disconnect (no msg popup)
		utils::hook::set<BYTE>(0x699B1C + 1, 0x03);

		// crashing snd func
		//utils::hook::nop(0x5DA453, 5);
#endif

		// RB_RenderThread :: stub placed onto 'Sys_StopRenderer'
		utils::hook(0x6EBE84, rb_renderthread_stub, HOOK_CALL).install()->quick();



		// fixes effects - calls RB_UpdateDynamicBuffers before R_IssueRenderCommands
		// - RB_UpdateDynamicBuffers is normally called from within RB_RenderThread (not active with r_smp_backend 0)
		// - now called from the main thread before calling R_IssueRenderCommands
		utils::hook(0x7A18AA, fix_dynamic_buffers, HOOK_CALL).install()->quick();

		// R_CreateDynamicBuffers :: increase tempskinbuffer (x2 - was 0x800000)
		utils::hook::set(0x709865 + 1, 0x1000000);
		utils::hook::set(0x6C9FB3 + 2, 0x1000000); // cmp max size in temp skinning func (warning)

		// composite testing
		//utils::hook(0x6D5908, RB_Draw3D_stub, HOOK_CALL).install()->quick();

		// no longer needed ?
		utils::hook::nop(0x6C7973, 5); // Sys_WaitWorkerCmdInternal - fx_marks_draw
		//utils::hook::nop(0x6C61D2, 5); // Sys_WaitWorkerCmdInternal - fx_draw


		// #
		// move the message loop from the renderer thread to the main thread
		
		utils::hook::nop(0x6EB2DE, 29); // RB_SwapBuffers :: nop peek msg
		utils::hook::set<BYTE>(0x6EB2FB, 0xEB); // RB_SwapBuffers :: skip translate and dispatch msg

		// second msg loop in rb_swapbuffers
		utils::hook::nop(0x6EB45E, 17); // RB_SwapBuffers :: nop peek msg
		utils::hook::set<BYTE>(0x6EB46F, 0xEB);

		// impl. msg loop in Sys_GetEvent
		utils::hook(0x867C5B, sys_getevent_stub, HOOK_JUMP).install()->quick();

#ifdef DEBUG_WINPROC // print window proc msg to console
		utils::hook(0x554B3E, main_win_proc_stub, HOOK_JUMP).install()->quick();
#endif

#if DEBUG
		// enable OutputDebugString --- oo
		utils::hook::nop(0x60AE2C, 2);
#endif


		// #
		// call worker cmd's directly without using jq threads - no longer needed

		//utils::hook(0x6CA0D4, add_skin_xmodel_cmd_stub, HOOK_JUMP).install()->quick();
		//utils::hook(0x659089, 5); // do not wait or flush callback ^

		//utils::hook::nop(0x6C74F5, 6);
		//utils::hook(0x6C74F5, add_sceneent_cmd_stub, HOOK_JUMP).install()->quick();
		//utils::hook::nop(0x6C7515, 3); // add esp, 0xC

		// fix random skinned model rendering order (caused smearing artifacts) by not using delayed skinning 
		// (R_AddDObjToScene -> CG_PredictiveSkinCEntity -> Add Worker CMD)
		// not sure whats different compared to older cods .. might just be the threading issues in bo1
		// https://github.com/NVIDIAGameWorks/rtx-remix/issues/450
		utils::hook::nop(0x6E2B50, 6);
		utils::hook(0x6E2B50, add_dobj_skin_cmd_stub, HOOK_JUMP).install()->quick();

		// 60B390 -  FX_GenerateVerts
		utils::hook::nop(0x6E2A43, 5);
		utils::hook(0x6E2A56, fx_draw_cmd_stub, HOOK_JUMP).install()->quick();

		// disable RB_CalcSunSpriteSamples on R_Init
		utils::hook::nop(0x6B833F, 10); 


		// #
		// pretess - r_pretess 1 drastically improves fps but results in unstable geo hashes

		// R_GenerateSortedDrawSurfs :: fix r_pretess by setting buffer->used to zero before starting to add stuff to the scene
		utils::hook::nop(0x6C70AC, 6);
		utils::hook(0x6C70AC, fix_r_pretess_stub, HOOK_JUMP).install()->quick(); 

		// ^ increase pretess buffers .. not needed but cant hurt ig
		utils::hook::set<BYTE>(0x70971A + 3, 0x20); // double -> also doubles dyn index and dyn vertex buffers
		utils::hook::set<BYTE>(0x7098D8 + 3, 0x40); // needs to be double of ^
		utils::hook::set<BYTE>(0x7095E3 + 3, 0x40); // needs to be double of ^ -> R_InitDynamicIndexBufferState

		// fix pretess deadlock - hack
		utils::hook::set<BYTE>(0x6C7089, 0xEB);


		// #
		// implement r_forcelod logic for skinned models (R_SkinXModel)
		utils::hook(0x523BD8, XModelGetLodForDist_stub, HOOK_JUMP).install()->quick();


		// #
		// playermodel shadow
		utils::hook::set<BYTE>(0x6C18EC, 0xEB); // render 3rd person model (R_AddDObjSurfacesCamera)
		utils::hook::nop(0x67F56B, 8); utils::hook(0x67F56B, reduce_playerheight_for_shadow_stub, HOOK_JUMP).install()->quick(); //utils::hook::set<BYTE>(0x67F550, 0xEB); // do not reduce height of 3rd person playermodel (CG_Player)
		utils::hook(0x67FA19, cg_player_stub, HOOK_CALL).install()->quick(); // set unique texture for all parts of the model incl. gun 


		// #
		// disable loading of specular and normalmaps (de-clutter remix ui)
		// * only enable for development reasons because it currently breaks streaming
		if (!game::is_game_mod && flags::has_flag("disable_normal_spec"))
		{
			utils::hook::nop(0x70A4CD, 5);
			utils::hook::nop(0x736E19, 7); utils::hook(0x736E19, load_image_stub, HOOK_JUMP).install()->quick();
			utils::hook(0x6F9064, load_image_stream_stub, HOOK_JUMP).install()->quick();

			utils::hook(0x725BC4, r_setsampler_stub, HOOK_JUMP).install()->quick();
		}

		// load_common_fast_files - not called when using game_mod
		//utils::hook(0x4C8966, load_common_fast_files, HOOK_CALL).install()->quick();
		utils::hook(0x50F41D, load_level_fastfile_hk, HOOK_CALL).install()->quick();
		utils::hook(0x51B175, load_level_fastfile_hk, HOOK_CALL).install()->quick();
		utils::hook(0x88A410, load_level_fastfile_hk, HOOK_CALL).install()->quick();

		// #
		// game_mod patches by nukem - https://github.com/Nukem9/LinkerMod/blob/development/components/game_mod/dllmain.cpp
		// only apply if game_mod isn't loaded
		if (!game::is_game_mod)
		{
			// Remove improper quit popup
			utils::hook::set(0x4F1930, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			// Bypass deletion of 'steam_appid.txt' - (keep debugger attached on proc startup)
			utils::hook::set(0x8B41AF, (PBYTE)"\x90\x90", 2);
			utils::hook::set(0x8B41A5, (PBYTE)"\x90\x90\x90\x90\x90", 5); // fopen
			utils::hook::set(0x8B41B3, (PBYTE)"\x90\x90\x90\x90\x90", 5); // fclose

			// Force sv_vac 0
			utils::hook::set(0x5A272C, (PBYTE)"\x00", 1);
			utils::hook::set(0x5A272E, (PBYTE)"\x00", 1);
			utils::hook::set(0x655051, (PBYTE)"\x90\x90\x90\x90\x90", 5);

			// Unrestrict Dvar_ForEachConsoleAccessName,
			// Cmd_ForEachConsoleAccessName, and Dvar_ListSingle
			utils::hook::nop(0x5F5D4E, 2);
			utils::hook::nop(0x610A94, 2);
			utils::hook::nop(0x82E63C, 6);

			// Enable the in-game console
			utils::hook(0x587DC8, Con_ToggleConsole, HOOK_CALL).install()->quick();
			utils::hook(0x587633, Con_ToggleConsole, HOOK_CALL).install()->quick();
			utils::hook::set<BYTE>(0x58761C, 0xEB);

			// unlock console commands
			utils::hook::nop(0x829B42, 5);

			// restore commandline functionality
			utils::hook::set(0x50A8C6, (PBYTE)"\x68\xD8\xD0\x76\x02", 5);

			utils::hook::set<BYTE>(0x4A3920 + 1, 0x01); // cg_fov

			// Stop vanilla scripts from resetting the user's fov
			utils::hook(0x62A0B0, dvars::Dvar_SetFromStringByName, HOOK_JUMP).install()->quick();
		}

		// *
		// general

		// disable the need for forward/backslashes for console cmds
		utils::hook::nop(0x51226D, 5);

		// dxvk's 'EnumAdapterModes' returns a lot of duplicates and the games array only has a capacity of 256 which is not enough depending on max res. and refreshrate
		// fix resolution issues by removing duplicates returned by EnumAdapterModes - then write the array ourselfs
		utils::hook(0x6B7294, sp::resolution::R_EnumDisplayModes_stub, HOOK_JUMP).install()->quick();
		utils::hook(0x6B72D6, sp::resolution::R_EnumDisplayModes_stub2, HOOK_JUMP).install()->quick();
		utils::hook::set<BYTE>(0x6B7282 + 2, 0x04); // set max array size check to 1024 (check within loop)

		// :*
		utils::hook::nop(0x6B6905, 5); utils::hook::nop(0x6B6910, 6);
		utils::hook(0x6B6910, fix_aspect_ratio_stub, HOOK_JUMP).install()->quick();

		// :* x2
		utils::hook::set(0x41EEC0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
		

		// ------------------------------------------------------------------------

#ifdef GIT_DESCRIBE
		static auto version_str = std::string("t5-rtx-"s + GIT_DESCRIBE + " > "s);
#else
		static auto version_str = "t5-rtx > "s;
#endif

		// console string ;)
		utils::hook::set<const char*>(0x79E806 + 1, version_str.c_str());

		command::add("borderless", [this](command::params)
		{
			const auto hwnd = game::main_window;

			// calculate titlebar height
			RECT w, c; GetWindowRect(hwnd, &w); GetClientRect(hwnd, &c);
			sp::main_module::noborder_titlebar_height = (w.bottom - w.top) - (c.bottom - c.top);

			SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
			SetWindowPos(hwnd, nullptr, 0, 0, game::sp::dx->windows->width, game::sp::dx->windows->height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
		});
	}
}
