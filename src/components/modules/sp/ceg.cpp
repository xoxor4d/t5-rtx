// all credits to Nukem: https://github.com/Nukem9/LinkerMod/blob/development/components/game_mod/CEG.cpp

#include "std_include.hpp"

namespace components::sp
{
	void* sub_8EE640(void* nullptr1, void* nullptr2)
	{
		__debugbreak();

		if (nullptr1 != nullptr || nullptr2 != nullptr)
		{
			__debugbreak();
		}
			
		*(void**)0xBA1C24 = nullptr2;
		return (void*)0xBA1C24;
	}

	ceg::ceg()
	{
		if (!game::is_game_mod)
		{
			// a few more ceg patches that game_mod does not need to handle because affected functions are detoured
			utils::hook::nop(0x7D905F, 5); // PlayerCmd_SetPerk (second one in this func, first is already handled)
			utils::hook::set(0x4DA5B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // ceg func that gets called by multiple other funcs
			utils::hook::nop(0x7D91CF, 5); // PlayerCmd_UnsetPerk
			utils::hook::set(0x502190, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // ceg func that gets called by multiple other funcs

			utils::hook::set(0x662F20, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Kill Steam DRM IPC creation
			utils::hook::set(0x662F20, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Kill Steam DRM IPC creation
			utils::hook::set(0x46C9A0, (PBYTE)"\xCC", 1);
			utils::hook::set(0x5F3290, (PBYTE)"\xCC", 1);

			utils::hook::set(0x4DFD60, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Registry key checks
			utils::hook::set(0x53F880, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5A5360, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4DFD60, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5A5360, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			utils::hook::set(0x64F6A0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Single function, 32bit hash check
			utils::hook::set(0x5614A0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x417360, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x56AB40, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x59BEB0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			utils::hook::set(0x676740, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Direct ExitProcess() check
			utils::hook::set(0x5DB020, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4F02C0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x572DF0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x679B40, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4BFB50, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4D4B80, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x501080, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5CAB50, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4C0DE0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			utils::hook::set(0x41CEB0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Wrapper ExitProcess() check, executes
			utils::hook::set(0x41CF50, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // on certain days of the week
			utils::hook::set(0x427E00, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x437350, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x444E80, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x449C30, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4545A0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x45C550, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x462B60, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4682B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x487D80, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x48C7B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4A9970, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4C3360, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4D9650, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x4E3860, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x515B20, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5268E0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x527200, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5474A0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x54F280, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x561410, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x59D820, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5A8800, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5B4330, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5D3F20, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x5EF2A0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x6099E0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x610A60, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x625500, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x625980, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x64FFB0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x662EC0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x662F80, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x671860, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x67B3B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x67D830, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x6857B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x68D300, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x68E8F0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			utils::hook::set(0x9A23B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Wrapper ExitProcess() check with HWBP detection,
			utils::hook::set(0x9A23F0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // executes on certain days of the week
			utils::hook::set(0x9A2430, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2470, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A24B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A24F0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2530, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2570, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A25B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A25F0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2630, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2670, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A26B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A26F0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2730, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A2770, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			utils::hook::set(0x9A2980, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6); // Direct HWBP check
			utils::hook::set(0x9A29B0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);
			utils::hook::set(0x9A29E0, (PBYTE)"\xB8\x01\x00\x00\x00\xC3", 6);

			utils::hook(0x8EE640, sub_8EE640, HOOK_JUMP).install()->quick();

			utils::hook::jump(0x60CC10, 0x004F20F0); // CEGObfuscate<LiveStats_Init> => LiveStats_Init
			utils::hook::jump(0x63DCC0, 0x0062DD40); // CEGObfuscate<LiveStats_ResetStats> => LiveStats_ResetStats
			utils::hook::jump(0x580460, 0x0079E6D0); // CEGObfuscate<Con_Restricted_SetLists> => Con_Restricted_SetLists
		}
	}
}
