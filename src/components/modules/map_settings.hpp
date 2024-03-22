#pragma once

namespace components
{
	class map_settings : public component
	{
	public:
		map_settings();
		~map_settings() = default;
		const char* get_name() override { return "map_settings"; }

		static map_settings* p_this;
		static map_settings* get();

		struct map_settings_s
		{
			std::string		mapname;
			float			fog_distance = 5000.0f;
			game::GfxColor	fog_color = { 0x00FFFFFF };
			game::vec3_t	sun_direction = { 75.0f, -15.0f, -35.0f };
			game::vec3_t	sun_color = { 255.0f, 255.0f, 255.0f };
			float			sun_intensity = 1.0f;
			int				sky_index = 2;
		};

		static inline const map_settings_s* settings() { return &m_loaded_map_settings; }
		void set_settings_for_loaded_map(bool reload_settings = false);

	private:
		static inline map_settings_s m_loaded_map_settings = {};
		static inline std::vector<map_settings_s> m_settings;
		bool load_settings();
	};
}
