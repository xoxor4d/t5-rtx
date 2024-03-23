#include "std_include.hpp"

namespace components
{
	map_settings* map_settings::p_this = nullptr;
	map_settings* map_settings::get() { return p_this; }

	void map_settings::set_settings_for_loaded_map(bool reload_settings)
	{
		DEBUG_PRINT("[T5RTX-DEBUG] # Function: map_settings::set_settings_for_loaded_map()\n");

		if ((m_settings.empty() || reload_settings) && !map_settings::load_settings())
		{
			return;
		}

		if (const auto rgp = SELECT(nullptr, game::sp::rgp);  // todo - mp
			rgp && rgp->world && rgp->world->name)
		{
			std::string map_name = rgp->world->name;
			utils::replace_all(map_name, std::string("maps/mp/"), "");	// if mp map
			utils::replace_all(map_name, std::string("maps/"), "");		// if sp map
			utils::replace_all(map_name, std::string(".d3dbsp"), "");

			for (const auto& s : m_settings)
			{
				if (s.mapname == map_name)
				{
					m_loaded_map_settings = s;
					break;
				}
			}
		}
	}

	constexpr auto INI_MAPNAME_ARG = 0;
	constexpr auto INI_FOG_DIST = 1;
	constexpr auto INI_FOG_COLOR_BEGIN = 2;
	constexpr auto INI_SUN_DIR_BEGIN = 5;
	constexpr auto INI_SUN_COLOR_BEGIN = 8;
	constexpr auto INI_SUN_INTENSITY = 11;
	constexpr auto INI_SKY_INDEX = 12;
	constexpr auto INI_ARGS_TOTAL = 13;

	bool map_settings::load_settings()
	{
		DEBUG_PRINT("[T5RTX-DEBUG] # Function: map_settings::load_settings()\n");

		m_settings.clear();
		m_settings.reserve(32);

		std::ifstream file;
		if (utils::fs::open_file_homepath("t5rtx", "map_settings.ini", false, file))
		{
			std::string input;
			std::vector<std::string> args;

			bool reading_cull_settings = false;

			// read line by line
			while (std::getline(file, input))
			{
				// ignore comment
				if (utils::starts_with(input, "//"))
				{
					continue;
				}

				if (!reading_cull_settings && utils::starts_with(input, "#CULL"))
				{
					reading_cull_settings = true;
					continue;
				}

				// split string on ','
				args = utils::split(input, ',');

				if (reading_cull_settings)
				{
					// check if there are map settings
					bool map_settings_exist = false;
					map_settings_s* s = nullptr;

					// check if map settings exist
					for (auto& e : m_settings)
					{
						if (e.mapname._Equal(args[INI_MAPNAME_ARG]))
						{
							s = &e;
							map_settings_exist = true;
							break;
						}
					}

					// create defaults if not
					if (!map_settings_exist)
					{
						m_settings.push_back(map_settings_s(args[INI_MAPNAME_ARG]));
						s = &m_settings.back();
					}

					if (s)
					{
						// for each cell with its forced indices with format -> [cell](index index index)
						for (auto a = 1u; a < args.size(); a++)
						{
							const auto& str = args[a];

							if (str.empty())
							{
								// print msg here
								continue;
							}

							// which cell are we writing settings for?
							const auto cell_index = utils::try_stoi(utils::split_string_between_delims(str, '[', ']'), -1);
							if (cell_index != -1)
							{
								// check for duplicate cells
								bool ignore_current_cell = false;
								for (const auto& c : s->cell_settings)
								{
									if (cell_index == c.cell_index)
									{
										ignore_current_cell = true;
										break;
									}
								}

								// cell has been added already
								if (ignore_current_cell)
								{
									// print msg here
									continue;
								}

								// get inidices
								const auto indices_str = utils::split_string_between_delims(str, '(', ')');
								const auto split_indices = utils::split(indices_str, ' ');

								s->cell_settings.push_back(cell_settings_s(cell_index));
								const auto c = &s->cell_settings.back();

								// for each forced index
								for (const auto& i : split_indices)
								{
									c->forced_cell_indices.push_back(utils::try_stoi(i, -1));
								}
							}
						}
					}
				}

				// map settings
				else
				{
					if (args.size() == INI_ARGS_TOTAL)
					{
						m_settings.push_back(
							{
								args[INI_MAPNAME_ARG],
								utils::try_stof(args[INI_FOG_DIST], 5000.0f),
								D3DCOLOR_XRGB
								(
									utils::try_stoi(args[INI_FOG_COLOR_BEGIN + 0], 255),
									utils::try_stoi(args[INI_FOG_COLOR_BEGIN + 1], 255),
									utils::try_stoi(args[INI_FOG_COLOR_BEGIN + 2], 255)
								),
								{
									utils::try_stof(args[INI_SUN_DIR_BEGIN + 0], 75.0f),
									utils::try_stof(args[INI_SUN_DIR_BEGIN + 1], -15.0f),
									utils::try_stof(args[INI_SUN_DIR_BEGIN + 2], -35.0f)
								},
								{
									utils::try_stof(args[INI_SUN_COLOR_BEGIN + 0], 255),
									utils::try_stof(args[INI_SUN_COLOR_BEGIN + 1], 255),
									utils::try_stof(args[INI_SUN_COLOR_BEGIN + 2], 255)
								},
								utils::try_stof(args[INI_SUN_INTENSITY], 1.0f),
								utils::try_stoi(args[INI_SKY_INDEX], 2)
							});
					}
				}
			}

			file.close();
			return true;
		}

		return false;
	}

	map_settings::map_settings()
	{
		command::add("mapsettings_update", [this](command::params)
		{
			map_settings::set_settings_for_loaded_map(true);
		});
	}
}