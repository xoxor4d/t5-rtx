#pragma once

namespace dvars
{
	extern game::dvar_s* fx_cull_elem_draw_radius;

	extern game::dvar_s* r_showTess;
	extern game::dvar_s* r_showTessSkin;
	extern game::dvar_s* r_showTessDist;
	extern game::dvar_s* r_showCellIndex;

	extern game::dvar_s* rtx_water_uv_scale;

	void bool_override(const char* dvarName, const bool& value, const game::dvar_flags& flags = game::saved);
	void int_override(const char* dvarName, const int& value, const game::dvar_flags& flags = game::saved, const bool& limits = false, const int& mins = 0, const int& maxs = 0);
	void float_override(const char* dvarName, const float& value, const game::dvar_flags& flags = game::saved, const bool& limits = false, const float& mins = 0.0f, const float& maxs = 0.0f);

	game::dvar_s* Dvar_SetFromStringByNameFromSource(const char* dvarName, const char* string, game::DvarSetSource source, unsigned int flags);
	void Dvar_SetFromStringByName(const char* dvarName, const char* string);
}