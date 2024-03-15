#pragma once

namespace components::sp
{
	class rtx : public component
	{
	public:
		rtx();
		~rtx() = default;
		const char* get_name() override { return "rtx_sp"; }

		static inline auto loc_disable_world_culling = 0u;
		static inline auto loc_disable_entity_culling = 0u;
		static inline const char* rtx_disable_world_culling_enum[] = { "default", "less", "all", "all-but-models" };
		//static inline std::vector rtx_disable_world_culling_enum = { "default", "less", "all", "all-but-models" };

		static void setup_rtx(game::GfxViewParms* view_parms);

		static void force_dvars_on_frame();
		static void set_dvar_defaults();
	};
}
