#pragma once

namespace components::sp
{
	class main_module : public component
	{
	public:
		main_module();
		~main_module() = default;
		const char* get_name() override { return "main_module_sp"; }

		static DWORD WINAPI find_window_loop(LPVOID);

		static void on_map_load();
		static void on_map_shutdown();

		static void RB_ShowTess(game::GfxCmdBufSourceState* source, game::GfxCmdBufState* state, const float* center, const char* name, const float* color);
		static inline IDirect3DTexture9* m_sky_texture = nullptr;

		static inline float m_reduce_playerheight_for_shadow = 0.0f;

	private:
		static inline bool noborder_active = false;
		static inline int noborder_titlebar_height = 0;
	};
}
