#include "std_include.hpp"

#define HIDWORD(x)  (*((DWORD*)&(x)+1))

using namespace game::sp;

namespace components::sp
{
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
			const static uint32_t retn_addr = 0x6D5CF2;
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
			const static uint32_t R_CompareDisplayModes_addr = 0x6D5C70;
			const static uint32_t retn_addr = 0x6D5D2E;
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

	void fix_aspect_ratio(int* window_parms)
	{
		*reinterpret_cast<float*>(0x3BED844) = static_cast<float>(window_parms[7]) / static_cast<float>(window_parms[8]);
	}

	void __declspec(naked) fix_aspect_ratio_stub()
	{
		const static uint32_t retn_addr = 0x6D504E;
		__asm
		{
			pop		eax;
			pushad;
			push	eax;
			call	fix_aspect_ratio;
			add		esp, 4;
			popad;
			push	eax;

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

#define DEBUG_WINPROC
#ifdef DEBUG_WINPROC
	void print_main_win_proc_msg(std::uint32_t msg)
	{
		//if (const auto var = game::sp::Dvar_FindVar("r_smp_backend"); var && !var->current.enabled)
		{
			printf("MSG: %d\n", msg);
		}
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

	// main thread
	void msg_loop()
	{
		//const auto hwnd = reinterpret_cast<HWND>(0x27706BC);
		HWND hwnd = nullptr;

		tagMSG msg = {};
		while (PeekMessageA(&msg, hwnd, 0, 0, 0))
		{
			if (!GetMessageA(&msg, hwnd, 0, 0))
			{
				// Sys_Quit
				utils::hook::call<void (__cdecl)()>(0x5DF2B0)();
			}

			//dword_27706D4 = msg.time;
			*(DWORD*)0x27706D4 = msg.time;

			TranslateMessage(&msg);
			DispatchMessageA(&msg);

			printf("Msg loop: %d \n", msg.message);
		}

		/*auto eventhead = reinterpret_cast<int*>(0x276D5DC);
		auto eventtail = reinterpret_cast<int*>(0x276F55C);

		auto eventque = reinterpret_cast<game::sysEvent_t*>(0x276DAF0);
		game::sysEvent_t xx[256] = {};

		memcpy(&xx, eventque, sizeof(game::sysEvent_t) * 256);
		auto aaaa = 9;*/
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
	void print_Sys_QueryD3DDeviceOKEvent(BOOL result)
	{
		if (!result)
		{
			printf("Sys_QueryD3DDeviceOKEvent :: returned FALSE\n");
		}
	}

	void __declspec(naked) Sys_QueryD3DDeviceOKEvent_stub()
	{
		__asm
		{
			neg     eax;
			sbb     eax, eax;
			inc     eax;

			pushad;
			push	eax;
			call	print_Sys_QueryD3DDeviceOKEvent;
			pop		eax;
			popad;

			retn;
		}
	}

	void R_PreTessBspDrawSurfs_check(int count)
	{
		if (count + game::sp::gfx_buf->preTessIndexBuffer->used > game::sp::gfx_buf->preTessIndexBuffer->total)
		{
			printf("R_PreTessBspDrawSurfs :: Exceeded preTessIndexBuffer usage\n");
		}
	}

	void __declspec(naked) R_PreTessBspDrawSurfs_stub()
	{
		const static uint32_t retn_addr = 0x72C95D;
		__asm
		{
			add     edx, eax;

			pushad;
			push	edx; // count
			call	R_PreTessBspDrawSurfs_check;
			pop		edx;
			popad;

			cmp     edx, [ecx + 4];
			jmp		retn_addr;
		}
	}
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
		

	int fuck_me()
	{
		utils::hook::call<void(__cdecl)()>(0x6B82E0)(); // R_Init
		return utils::hook::call<int(__cdecl)()>(0x49D640)(); // related to g_connectpaths
	}

	void fuck_me2()
	{
		// TODO not needed ig
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

	void fuck_me3()
	{
		// og call - R_EndFrame
		utils::hook::call<void(__cdecl)()>(0x6D7B90)();

		//if (game::sp::dx)

		// TODO: call RB_UpdateDynamicBuffers (0x6EBBB0) -> assign frontentdata before that to 0x4643FD8 ---   before R_IssueRenderCommands !??!?

		// 0x4643FD8
		auto smpdata = reinterpret_cast<game::GfxBackEndData**>(0x4643FD8);
		const auto xx = game::sp::get_frontenddata_out();

		if (xx->viewInfo && xx->skinnedCacheVb)
		{
			*smpdata = xx;

			// RB_UpdateDynamicBuffers
			utils::hook::call<void(__cdecl)()>(0x6EBBB0)();
		}
	}


	main_module::main_module()
	{
		// setting sys_smp_allowed to 0 helps with the initial startup lock
		// R_ToggleSmpFrameCmd :: R_ToggleSmpFrame called before Sys_WakeRenderer


		// needs +set sys_smp_allowed 1 +set r_smp_worker 0 +set r_smp_backend 0 +set r_pretess 0 +set com_introPlayed 1 +set com_startupIntroPlayed 1 +devmap zombie_theater


		// do not call r_init from the renderer thread
		utils::hook::nop(0x6EBEC9, 5);

		// call r_init from the main thread
		// todo: reimpl. R_Init in various other functions
		utils::hook(0x52F28A, fuck_me, HOOK_CALL).install()->quick();

		utils::hook(0x6EBE84, fuck_me2, HOOK_CALL).install()->quick();

		// TODO: call RB_UpdateDynamicBuffers (0x6EBBB0) -> assign frontentdata before that to 0x4643FD8 ---   before R_IssueRenderCommands !??!?
		utils::hook(0x7A18AA, fuck_me3, HOOK_CALL).install()->quick();

		//utils::hook::nop(0x6D5853, 2); // R_HandOffToBackend :: disable sys_smp_allowed check for Sys_FrontEndSleep
		//utils::hook::nop(0x6D578A, 2); // R_SyncRenderThread :: same ^
		//utils::hook::nop(0x6B8ADF, 2); // same ^
		//utils::hook::nop(0x6C3882, 2); // same ^
		//utils::hook::nop(0x6D436A, 2); // same ^
		//utils::hook::nop(0x6D7E7E, 2); // same ^ - bad
		//utils::hook::nop(0x6D7EE6, 6); // same ^ - bad

		//utils::hook::nop(0x6D7FDE, 2); // same ^ .. hmm db related
		//utils::hook::nop(0x6E2A98, 2); // same ^ ... fx should not nop here?

		//utils::hook::nop(0x70C42A, 2); // same ^
		//utils::hook::nop(0x6D5912, 14); // R_IssueRenderCommands :: if ( Sys_IsRenderThread() ) Sys_RenderCompleted();

		// ouch
		//utils::hook::nop(0x6EB214, 6);
		//utils::hook::jump(0x6EB228, 0x6EB51D);

		//utils::hook::nop(0x6E2AA4, 5);



		/*try impl:
		while ( !R_FinishedFrontendWorkerCmds() )
		{
		  NET_Sleep(1u);
		}

		OR

		if ( Sys_IsMainThread() )
		{
			R_WaitFrontendWorkerCmds();
		}*/

		//utils::hook::nop(0x6EBEB3, 5);
		//utils::hook::set<BYTE>(0x59BA6C, 0xEB);


		utils::hook::nop(0x6C7973, 5); // Sys_WaitWorkerCmdInternal - fx_marks_draw
		//utils::hook::nop(0x6C61D2, 5); // Sys_WaitWorkerCmdInternal - fx_draw


		



#define MOVE_MSG_PUMP
#ifdef MOVE_MSG_PUMP
		utils::hook::nop(0x6EB2DE, 29); // RB_SwapBuffers :: nop peek msg
		utils::hook::set<BYTE>(0x6EB2FB, 0xEB); // RB_SwapBuffers :: skip translate and dispatch msg

		// second msg loop in rb_swapbuffers
		utils::hook::nop(0x6EB45E, 17); // RB_SwapBuffers :: nop peek msg
		utils::hook::set<BYTE>(0x6EB46F, 0xEB);

		//utils::hook::nop(0x6EB2B3, 43); // RB_SwapBuffers :: nop showcursor loop
		//utils::hook::nop(0x6EB288, 16); // RB_SwapBuffers :: nop sys_queryrenderevent
		//utils::hook::set<BYTE>(0x6EB298, 0xEB); // ^ skip more

		// impl. msg loop in Sys_GetEvent
		utils::hook(0x867C5B, sys_getevent_stub, HOOK_JUMP).install()->quick();
#endif


		// call worker cmd's directly without using jq threads

		//utils::hook(0x6CA0D4, add_skin_xmodel_cmd_stub, HOOK_JUMP).install()->quick();
		//utils::hook(0x659089, 5); // do not wait or flush callback ^

		//utils::hook::nop(0x6C74F5, 6);
		//utils::hook(0x6C74F5, add_sceneent_cmd_stub, HOOK_JUMP).install()->quick();
		//utils::hook::nop(0x6C7515, 3); // add esp, 0xC

		// 60B390 -  FX_GenerateVerts
		utils::hook::nop(0x6E2A43, 5);
		utils::hook(0x6E2A56, fx_draw_cmd_stub, HOOK_JUMP).install()->quick();

		// --

		// not needed
		//utils::hook::nop(0x6B833F, 10); // disable RB_CalcSunSpriteSamples


#ifdef DEBUG_WINPROC
		utils::hook(0x554B3E, main_win_proc_stub, HOOK_JUMP).install()->quick();
#endif

#if DEBUG
		// not the issue
		//utils::hook(0x4C9A1E, Sys_QueryD3DDeviceOKEvent_stub, HOOK_JUMP).install()->quick();

		// check failing 'if ( count + gfx_buf.preTessIndexBuffer->used > gfx_buf.preTessIndexBuffer->total )' check in R_PreTessBspDrawSurfs
		utils::hook(0x72C958, R_PreTessBspDrawSurfs_stub, HOOK_JUMP).install()->quick();
#endif

		// R_GenerateSortedDrawSurfs :: fix r_pretess by setting buffer->used to zero before starting to add stuff to the scene
		utils::hook::nop(0x6C70AC, 6);
		utils::hook(0x6C70AC, fix_r_pretess_stub, HOOK_JUMP).install()->quick(); 

		// ^ increase pretess buffers .. not needed but cant hurt ig
		utils::hook::set<BYTE>(0x70971A + 3, 0x20); // double -> also doubles dyn index and dyn vertex buffers
		utils::hook::set<BYTE>(0x7098D8 + 3, 0x40); // needs to be double of ^
		utils::hook::set<BYTE>(0x7095E3 + 3, 0x40); // needs to be double of ^ -> R_InitDynamicIndexBufferState



#if DEBUG
		// enable OutputDebugString --- oo
		utils::hook::nop(0x60AE2C, 2);
#endif

		// not needed?
		//utils::hook::set<BYTE>(0x4E79D0, 0xEB); // use 1 worker thread


		//utils::hook::set<BYTE>(0x6CAD89 + 1, 0x0); // min amount of workers
		//utils::hook::set<BYTE>(0x82D1FD + 1, 0x0); // ^ min amount of workers
		


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

		// precaching xmodels beyond level load (dynamic skysphere spawning)
		//utils::hook::set<BYTE>(0x54A4D6, 0xEB);

		// dxvk's 'EnumAdapterModes' returns a lot of duplicates and the games array only has a capacity of 256 which is not enough depending on max res. and refreshrate
		// fix resolution issues by removing duplicates returned by EnumAdapterModes - then write the array ourselfs
		//utils::hook(0x6D5CE2, sp::resolution::R_EnumDisplayModes_stub, HOOK_JUMP).install()->quick();
		//utils::hook(0x6D5D29, sp::resolution::R_EnumDisplayModes_stub2, HOOK_JUMP).install()->quick();
		//utils::hook::set<BYTE>(0x6D5CD0 + 2, 0x04); // set max array size check to 1024 (check within loop)

		// Remove Impure client (iwd) check - needed?
		//utils::hook::nop(0x5DBC7F, 30);

		// stuck in some loop 'Com_Quit_f'
		//utils::hook::nop(0x5FEA01, 5);

		// don't play intro video - allows to devmap into a map via commandline
		//utils::hook::nop(0x59D68B, 5); // mp: 0x564CB9

		// :*
		//utils::hook(0x6D4FB3, fix_aspect_ratio_stub, HOOK_JUMP).install()->quick();

		// ------------------------------------------------------------------------

		// un-cheat + userinfo flag for fx_enable
		//utils::hook::set<BYTE>(0x4A4D16 + 1, 0x01); // was 0x80

		// un-cheat + userinfo flag for sv_cheats
		//utils::hook::set<BYTE>(0x70B92D + 1, 0x01); // was 0x48
		//utils::hook::set<BYTE>(0x70B92D + 1, 0x01);

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

		/*command::add("windowed", [this](command::params)
		{
			if (sp::main_module::noborder_titlebar_height)
			{
				const auto hwnd = game::main_window;
				SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
				SetWindowPos(hwnd, nullptr, 0, 0, game::sp::dx->windows->width, game::sp::dx->windows->height + sp::main_module::noborder_titlebar_height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
			}
		});*/
	}
}
