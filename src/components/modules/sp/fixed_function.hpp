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

		struct vb_vert_range_s
		{
			std::uint32_t base_offset;
			std::uint32_t size;
		};

		static inline std::vector<vb_vert_range_s> modified_vertices_list;

		static void build_worldmatrix_for_object(float (*mtx)[4], const float* quat, const float* origin, const float scale);
		static void build_worldmatrix_for_object(float (*mtx)[4], float(*model_axis)[3], const float* origin, const float scale);
		static bool render_sw4_dual_blend(const game::GfxCmdBufState* state, const game::XSurface* surf, std::uint32_t vertex_offset);

	private:
	};
}
