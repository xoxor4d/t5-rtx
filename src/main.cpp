#include "std_include.hpp"

// https://github.com/Nukem9/LinkerMod/blob/72ee05bbf42dfb2a1893e655788b631be63ea317/components/game_mod/dllmain.cpp#L9
bool check_for_german_binary()
{
	// The location of the PDB string in the **German binary**
	// Note: This will point to arbitrary data in any other version
	const char* pdb_str_offset = (const char*)0xB61694;

	// Target PDB string for the **German binary**
	const char pdb[] = R"(C:\projects_pc\cod\codsrc\src\obj\t5\CoDSteam_CEG_German_bin\BlackOps.pdb)";
	const size_t len = _countof(pdb);

	DWORD dwProtect = 0;
	VirtualProtect((LPVOID)pdb_str_offset, len + 1, PAGE_EXECUTE_READ, &dwProtect);

	const bool result = strncmp(pdb_str_offset, pdb, len) == 0;

	VirtualProtect((LPVOID)pdb_str_offset, len + 1, dwProtect, &dwProtect);

	return result;
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// check if game_mod is loaded
		game::is_game_mod = *reinterpret_cast<BYTE*>(0x8B41AF) == 0x90;

#if DEBUG
		// disable stdout buffering
		setvbuf(stdout, nullptr, _IONBF, 0);

		// create an external console
		if (AllocConsole())
		{
			FILE* file = nullptr;
			freopen_s(&file, "CONIN$", "r", stdin);
			freopen_s(&file, "CONOUT$", "w", stdout);
			freopen_s(&file, "CONOUT$", "w", stderr);
			SetConsoleTitleA("t5-rtx console");
		}
#endif

		char filename[MAX_PATH];
		GetModuleFileNameA(nullptr,filename, MAX_PATH);

		if (const auto pos = std::string_view(filename).find_last_of('\\'); 
			pos != std::string::npos)
		{
			const auto exe_name = std::string_view(filename).substr(pos + 1) ;
			game::is_sp = exe_name == "t5sp.exe" || exe_name == "BlackOps.exe";
			game::is_mp = exe_name == "t5mp.exe" || exe_name == "BlackOpsMp.exe";

			if (game::is_mp)
			{
				return FALSE;
			}

			ASSERT(!check_for_german_binary(), "German version is not supported %s", "!");
		}

		components::loader::initialize();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		components::loader::uninitialize();
	}

	return TRUE;
}
