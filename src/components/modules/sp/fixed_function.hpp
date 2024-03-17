#pragma once

namespace components::sp
{
	class fixed_function : public component
	{
	public:
		fixed_function();
		~fixed_function() = default;
		const char* get_name() override { return "fixed_function_sp"; }

		static inline IDirect3DBaseTexture9* last_valid_sky_texture = nullptr;

	private:
	};
}
