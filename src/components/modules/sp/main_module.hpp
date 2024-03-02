#pragma once

namespace components::sp
{
	class main_module : public component
	{
	public:
		main_module();
		~main_module() = default;
		const char* get_name() override { return "main_module_sp"; }

		static void on_map_load();
		static void on_map_shutdown();

	private:
		static inline bool noborder_active = false;
		static inline int noborder_titlebar_height = 0;
	};
}