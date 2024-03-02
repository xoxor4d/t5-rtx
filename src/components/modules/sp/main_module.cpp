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

	// *
	// Event stubs

	// > fixed_function::init_fixed_function_buffers_stub
	void main_module::on_map_load()
	{
		map_settings::get()->set_settings_for_loaded_map();
	}

	// > fixed_function::free_fixed_function_buffers_stub
	void main_module::on_map_shutdown()
	{
	}

	main_module::main_module()
	{
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
