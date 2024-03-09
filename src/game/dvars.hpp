#pragma once

namespace dvars
{
	//extern game::dvar_s* fx_cull_elem_draw_radius;

	void bool_override(const char* dvarName, const bool& value, const game::dvar_flags& flags = game::saved);
	void int_override(const char* dvarName, const int& value, const game::dvar_flags& flags = game::saved, const bool& limits = false, const int& mins = 0, const int& maxs = 0);
	void float_override(const char* dvarName, const float& value, const game::dvar_flags& flags = game::saved, const bool& limits = false, const float& mins = 0.0f, const float& maxs = 0.0f);
}