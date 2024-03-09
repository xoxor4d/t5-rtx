#pragma once

namespace components::sp
{
	class rtx : public component
	{
	public:
		rtx();
		~rtx() = default;
		const char* get_name() override { return "rtx_sp"; }

		static void setup_rtx(game::GfxViewParms* view_parms);

		static void force_dvars_on_frame();
		static void set_dvar_defaults();
	};
}
