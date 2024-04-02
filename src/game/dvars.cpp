#include "std_include.hpp"

namespace dvars
{
	game::dvar_s* fx_cull_elem_draw_radius = nullptr;

	game::dvar_s* r_showTess = nullptr;
	game::dvar_s* r_showTessSkin = nullptr;
	game::dvar_s* r_showTessDist = nullptr;
	game::dvar_s* r_showCellIndex = nullptr;

	game::dvar_s* rtx_water_uv_scale = nullptr;

	void bool_override(const char* dvarName, const bool& value, const game::dvar_flags& flags)
	{
		if (const auto& dvar = game::sp::Dvar_FindVar(dvarName); dvar)
		{
			dvar->current.enabled = value;
			dvar->latched.enabled = value;
			dvar->flags = flags;
			dvar->modified = false;
		}
	}

	void int_override(const char* dvarName, const int& value, const game::dvar_flags& flags, const bool& limits, const int& mins, const int& maxs)
	{
		if (const auto& dvar = game::sp::Dvar_FindVar(dvarName); dvar)
		{
			dvar->current.integer = value;
			dvar->latched.integer = value;
			dvar->flags = flags;
			dvar->modified = false;

			if (limits)
			{
				dvar->domain.integer.min = mins;
				dvar->domain.integer.max = maxs;
			}
		}
	}

	void float_override(const char* dvarName, const float& value, const game::dvar_flags& flags, const bool& limits, const float& mins, const float& maxs)
	{
		if (const auto& dvar = game::sp::Dvar_FindVar(dvarName); dvar)
		{
			dvar->current.value = value;
			dvar->latched.value = value;
			dvar->flags = flags;
			dvar->modified = false;

			if (limits)
			{
				dvar->domain.value.min = mins;
				dvar->domain.value.max = maxs;
			}
		}
	}

	// https://github.com/Nukem9/LinkerMod/blob/72ee05bbf42dfb2a1893e655788b631be63ea317/components/game_mod/dvar.cpp#L161
	game::dvar_s* Dvar_SetFromStringByNameFromSource(const char* dvarName, const char* string, game::DvarSetSource source, unsigned int flags)
	{
		return ((game::dvar_s * (__cdecl*)(const char*, const char*, game::DvarSetSource, unsigned int))0x00426820)(dvarName, string, source, flags);
	}

	void Dvar_SetFromStringByName(const char* dvarName, const char* string)
	{
		// Do not allow the default FOV to be set. Generally sent with CG_DeployServerCommand.
		if (dvarName && string)
		{
			if (!_stricmp(dvarName, "cg_fov") && !_stricmp(string, "65"))
				return;

			if (!_stricmp(dvarName, "cg_default_fov") && !_stricmp(string, "65"))
				return;
		}

		Dvar_SetFromStringByNameFromSource(dvarName, string, game::DVAR_SOURCE_INTERNAL, 0);
	}
}