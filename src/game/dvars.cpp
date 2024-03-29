#include "std_include.hpp"

namespace dvars
{
	game::dvar_s* fx_cull_elem_draw_radius = nullptr;
	game::dvar_s* rtx_disable_world_culling = nullptr;
	game::dvar_s* rtx_disable_entity_culling = nullptr;
	game::dvar_s* rtx_sky_follow_player = nullptr;

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
}