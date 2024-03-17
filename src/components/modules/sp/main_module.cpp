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

//#define SEAMLESS_VID_RESTART_TEST
#ifdef SEAMLESS_VID_RESTART_TEST
	void vid_restart_stub()
	{
		// R_ResizeWindow
		utils::hook::call<void(__cdecl)()>(0x6B7E10)();
	}

	void vid_restart_post_stub()
	{

		// R_ResizeWindow
		//utils::hook::call<void(__cdecl)()>(0x6B7E10)(); // calling it here "fixes" ui but mouse is still fucked and so is aspectratio

		// CL_SetupViewport
		utils::hook::call<void(__cdecl)()>(0x481F70)();

		// setting resizeWindow to true will trigger the logic in 'check_reset' below (running in 'R_IssueRenderCommands')
		//game::sp::dx->resizeWindow = true;

		const auto ui = reinterpret_cast<game::uiInfo_s*>(0x256AA50);
		utils::hook::call<void(__cdecl)(int* width, int* height, float* aspect)>(0x4B3720)(&ui->uiDC.screenWidth, &ui->uiDC.screenHeight, &ui->uiDC.screenAspect);
		ui->uiDC.bias = (ui->uiDC.screenWidth * 480 <= ui->uiDC.screenHeight * 640) ? 0.0f : (static_cast<std::float_t>(ui->uiDC.screenWidth) - static_cast<std::float_t>(ui->uiDC.screenHeight) * 1.3333334f) * 0.5f;

		/*const auto clsvid = reinterpret_cast<game::vidConfig_t*>(0x2EE750C);
		const auto vid = reinterpret_cast<game::vidConfig_t*>(0x3966148);

		const auto scr_full_unsave = reinterpret_cast<game::ScreenPlacement*>(0xC78E90);
		const auto scr_full = reinterpret_cast<game::ScreenPlacement*>(0xC78E18);
		const auto scr_view = reinterpret_cast<game::ScreenPlacement*>(0xC78DA0);*/

		/*game::sp::gfxCmdBufSourceState->sceneViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->sceneViewport.height = (int)vid->displayHeight;
		game::sp::gfxCmdBufState->viewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufState->viewport.height = (int)vid->displayHeight;*/

		//game::sp::gfxCmdBufSourceState->renderTargetWidth = (int)vid->displayWidth;
		//game::sp::gfxCmdBufSourceState->renderTargetHeight = (int)vid->displayHeight;
		//game::sp::dx->windows->width = (int)vid->displayWidth;
		//game::sp::dx->windows->height = (int)vid->displayHeight;

		/*game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.cullViewInfo.sceneViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.cullViewInfo.sceneViewport.height = (int)vid->displayHeight;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.cullViewInfo.displayViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.cullViewInfo.displayViewport.height = (int)vid->displayHeight;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.cullViewInfo.scissorViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.cullViewInfo.scissorViewport.height = (int)vid->displayHeight;*/

		/*game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.s1.sceneViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.s1.sceneViewport.height = (int)vid->displayHeight;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.s1.displayViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.s1.displayViewport.height = (int)vid->displayHeight;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.s1.scissorViewport.width = (int)vid->displayWidth;
		game::sp::gfxCmdBufSourceState->u.input.data->viewInfo->u0.s1.scissorViewport.height = (int)vid->displayHeight;*/

		// R_RecoverLostDevice
		//utils::hook::call<void(__cdecl)()>(0x6B89C0)();
	}

	int check_reset()
	{
		if (game::sp::dx->resizeWindow)
		{
			utils::hook::call<void(__cdecl)()>(0x6B88C0)(); // R_ReleaseLostDeviceAssets
			utils::hook::call<void(__cdecl)()>(0x6B70C0)(); // R_ReleaseForShutdownOrReset

			// R_RecoverLostDevice
			const auto ret = utils::hook::call<bool(__cdecl)()>(0x6B89C0)();

			// never reaching the point that R_RecoverLostDevice returns true because the reset fails
			// game crashes in a worker thread before
			if (ret)
			{
				game::sp::dx->resizeWindow = false;
			}

			//utils::hook::call<void(__cdecl)()>(0x658F70)(); // R_WaitFrontendWorkerCmds
			utils::hook::call<void(__cdecl)()>(0x47E3E0)(); // safely flush all workers

			if (game::sp::gfx_buf->skinnedCacheLockAddr)
			{
				game::sp::gfx_buf->skinnedCacheLockAddr = 0;

				// R_ToggleSmpFrame
				utils::hook::call<void(__cdecl)()>(0x6D5AC0)();
			}

			return 0;
		}

		return 1;
	}

	__declspec(naked) void r_issuerendercmds_stub()
	{
		const static uint32_t skip_addr = 0x6D5938;
		const static uint32_t retn_addr = 0x6D58A0;
		__asm
		{
			pushad;
			call	check_reset;
			cmp		eax, 1;
			je		OG_LOGIC;

			popad;
			jmp		skip_addr;

		OG_LOGIC:
			popad;
			// og
			mov[ecx + 0x16CF14], eax;
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
		//const auto& r_warm_static = game::sp::Dvar_FindVar("r_warm_static");

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

	void main_module::setup_sky_image(game::GfxImage* skyimg)
	{
		if (!main_module::m_sky_texture)
		{
			if (skyimg && skyimg->texture.cubemap)
			{
				const auto dev = game::sp::dx->device;

				D3DSURFACE_DESC desc;
				skyimg->texture.cubemap->GetLevelDesc(0, &desc);

				// create texture with same desc
				dev->CreateTexture(desc.Width, desc.Height, 1, desc.Usage, desc.Format, desc.Pool, &main_module::m_sky_texture, nullptr);

				D3DLOCKED_RECT lockedRect1;
				skyimg->texture.cubemap->LockRect(D3DCUBEMAP_FACE_POSITIVE_X, 0, &lockedRect1, nullptr, 0);

				IDirect3DSurface9* cube_surf;
				skyimg->texture.cubemap->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &cube_surf);

				IDirect3DSurface9* sky_surf;
				main_module::m_sky_texture->GetSurfaceLevel(0, &sky_surf);


				dev->UpdateSurface(cube_surf, nullptr, sky_surf, NULL);

				//dev->StretchRect(cube_surf, nullptr, sky_surf, nullptr, D3DTEXF_LINEAR);

				cube_surf->Release();
				sky_surf->Release();
				skyimg->texture.cubemap->UnlockRect(D3DCUBEMAP_FACE_POSITIVE_X, 0);
				//RECT sourceRect;
				//dev->UpdateSurface(&lockedRect, &sourceRect, sky_surf, NULL);
			}
		}
	}

	// *
	// Event stubs

	// > fixed_function::init_fixed_function_buffers_stub
	void main_module::on_map_load()
	{
		map_settings::get()->set_settings_for_loaded_map();
		rtx::set_dvar_defaults();

		if (main_module::m_sky_texture)
		{
			main_module::m_sky_texture->Release();
			main_module::m_sky_texture = nullptr;
		}

		fixed_function::fixed_function::last_valid_sky_texture = nullptr;
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

		//// R_UI3D_SetupTextureWindow (we need to scale Y * 2 because its rendered at 1024x512 and it doesnt auto fit the screen)
		//utils::hook::call<void(__cdecl)(int window_index, float x, float y, float w, float h)>(0x6E21B0)(0, 0.0f, 0.0f, 1.0f, 2.0f);

		//// RB_UI3D_RenderToTexture
		//utils::hook::call<void(__cdecl)(const void* cmds, game::GfxUI3DBackend* rbUI3D, game::GfxCmdBufInput* input)>(0x6E26A0)(view->cmds, &view->rbUI3D, &view->input);
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
			&& std::string_view(material->textureTable->u.image->name) != "default")
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


		//if (utils::starts_with(mat.current_material->info.name, "wc/sky_"))
		//{
		//	mat.technique_type = game::TECHNIQUE_UNLIT;
		//	const auto ui3d_tex = game::sp::gfxCmdBufSourceState->u.input.codeImages[game::TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP]->texture.basemap;
		//	if (ui3d_tex)
		//	{
		//		game::sp::dx->device->SetTexture(0, ui3d_tex);
		//	}
		//	//switch_material(&mat, "mc/mtl_test_sphere_silver");
		//}

		if (state->material->info.sortKey == 5)
		{
			if (state->material->textureTable->u.image->mapType == 5)
			{
				// maptype 5 = cube so load mat 'mc/mtl_skybox_sp_kowloon' instead?
				mat.technique_type = game::TECHNIQUE_UNLIT;
				switch_material(&mat, "mc/mtl_skybox_sp_kowloon");

				//if (switch_material(&mat, "mc/mtl_skybox_sp_kowloon"))
				//{
					//if (!switch_material(&mat, "identityNormalMap"))
					//{
					//	int x = 1;
					//	//if (const auto identity = game::sp::gfxCmdBufSourceState->u.input.codeImages[game::TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP];
					//	//	identity && identity->texture.basemap)
					//	//{
					//	//	state->material->textureTable->u.image = identity;
					//	//	//game::sp::dx->device->SetTexture(0, identity->texture.basemap);
					//	//}
					//}
				//}
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

			//const auto skyimg = reinterpret_cast<game::GfxImage**>(0x408930C);
			//game::sp::dx->device->SetTexture(0, skyimg[0]->texture.basemap);

			/*if (main_module::m_sky_texture)
			{
				game::sp::dx->device->SetTexture(0, main_module::m_sky_texture);
			}*/
		}

		state->origTechType = state->techType;
		state->techType = mat.technique_type;

		return 1;
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
				info[i].allocFlags = 256;
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
			info[i].allocFlags = 256;
			info[i].freeFlags = 0;
			++i;
		}

		//DB_LoadXAssets(info, 1u, 0);
		utils::hook::call<void(__cdecl)(game::XZoneInfo*, std::uint32_t, int)>(0x631B10)(info, i, 0);
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


#ifdef SEAMLESS_VID_RESTART_TEST
		// vid restart tries
		utils::hook::set<BYTE>(0x6EB28F, 0xEB);
		utils::hook::nop(0x6B7E01, 2); // do not sleep in R_CheckResizeWindow
		utils::hook::nop(0x6EBF14, 5); // do not call 'RB_SwapBuffers' in RB_RenderThread

		// do not call 'R_ResizeWindow' in RB_SwapBuffers
		utils::hook::set<BYTE>(0x6EB24C, 0xEB);
		utils::hook::nop(0x6EB20D, 2);

		utils::hook(0x5B52B8, vid_restart_stub, HOOK_CALL).install()->quick(); // vid_restart
		utils::hook(0x5B52DA, vid_restart_post_stub, HOOK_CALL).install()->quick(); // vid_restart

		utils::hook(0x5D2F2C, vid_restart_stub, HOOK_CALL).install()->quick(); // vid_restart_complete
		utils::hook(0x5D2F4B, vid_restart_post_stub, HOOK_CALL).install()->quick(); // vid_restart_complete

		utils::hook::nop(0x6D589A, 6);
		utils::hook(0x6D589A, r_issuerendercmds_stub, HOOK_JUMP).install()->quick(); // R_IssueRenderCommands
#endif


		// RB_RenderThread :: stub placed onto 'Sys_StopRenderer'
		utils::hook(0x6EBE84, rb_renderthread_stub, HOOK_CALL).install()->quick();


		// fixes effects - calls RB_UpdateDynamicBuffers before R_IssueRenderCommands
		// - RB_UpdateDynamicBuffers is normally called from within RB_RenderThread (not active with r_smp_backend 0)
		// - now called from the main thread before calling R_IssueRenderCommands
		utils::hook(0x7A18AA, fix_dynamic_buffers, HOOK_CALL).install()->quick();


		// no longer needed
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
		// disable loading of specular and normalmaps (de-clutter remix ui)
		// * only enable for development reasons because it currently breaks streaming
		if (!game::is_game_mod && flags::has_flag("disable_normal_spec"))
		{
			utils::hook::nop(0x70A4CD, 5);
			utils::hook::nop(0x736E19, 7); utils::hook(0x736E19, load_image_stub, HOOK_JUMP).install()->quick();
			utils::hook(0x6F9064, load_image_stream_stub, HOOK_JUMP).install()->quick();

			utils::hook(0x725BC4, r_setsampler_stub, HOOK_JUMP).install()->quick();
		}

		// load_common_fast_files
		utils::hook(0x4C8966, load_common_fast_files, HOOK_CALL).install()->quick();


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
