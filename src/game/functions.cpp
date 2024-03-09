#include "std_include.hpp"

namespace game
{
	bool is_sp = false;
	bool is_mp = false;

	std::vector<std::string> loaded_modules;
	HWND main_window = nullptr;

	// true if game_mod is loaded
	bool is_game_mod = false;

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	namespace sp
	{
		DxGlobals* dx = reinterpret_cast<DxGlobals*>(0x3963440);
		//clipMap_t* cm = reinterpret_cast<clipMap_t*>(0x1F41A00);
		r_global_permanent_t* rgp = reinterpret_cast<r_global_permanent_t*>(0x3966180);
		GfxCmdBufSourceState* gfxCmdBufSourceState = reinterpret_cast<GfxCmdBufSourceState*>(0x40CA570);

		// GfxWorldDraw* g_worldDraw = reinterpret_cast<GfxWorldDraw*>(0x460C0B0);
		GfxWorldDraw* get_g_world_draw()
		{
			const auto out = reinterpret_cast<GfxWorldDraw*>(*game::sp::g_world_draw_ptr);
			return out;
		}

		game::GfxBackEndData* get_frontenddata_out()
		{
			const auto out = reinterpret_cast<game::GfxBackEndData*>(*game::sp::frontenddata_out_ptr);
			return out;
		}

		game::GfxBuffers* gfx_buf = reinterpret_cast<game::GfxBuffers*>(0x41706E0);

		cmd_function_s** cmd_ptr = reinterpret_cast<cmd_function_s**>(0x2430014);

		CmdArgs* cmd_args()
		{
			const auto cmd_args = static_cast<CmdArgs*>(Sys_GetValue(4));
			return cmd_args;
		}

		int cmd_argc()
		{
			const auto cmd = cmd_args();
			return cmd->argc[cmd->nesting];
		}

		const char* cmd_argv(int arg_index)
		{
			const auto cmd = cmd_args();

			if (arg_index >= cmd->argc[cmd->nesting])
			{
				return "";
			}

			return cmd->argv[cmd->nesting][arg_index];
		}


		Cmd_ExecuteSingleCommand_t	Cmd_ExecuteSingleCommand = Cmd_ExecuteSingleCommand_t(0x619D00);
		Cbuf_AddText_t	Cbuf_AddText = Cbuf_AddText_t(0x49B930);
		Com_PrintMessage_t Com_PrintMessage = Com_PrintMessage_t(0x43C520);


		//Com_Error_t Com_Error = Com_Error_t(0x59AC50);
		//DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile = DB_EnumXAssets_FastFile_t(0x48D560);
		//DB_LoadXAssets_t DB_LoadXAssets = DB_LoadXAssets_t(0x48E7B0);

		//scr_const_t* scr_const = reinterpret_cast<scr_const_t*>(0x1F33B90);

		//void Cbuf_AddText(const char* text /*eax*/)
		//{
		//	const static uint32_t Cbuf_AddText_func = 0x594200;
		//	__asm
		//	{
		//		mov		ecx, 0;
		//		mov		eax, text;
		//		call	Cbuf_AddText_func;
		//	}
		//}

		//void Vec2UnpackTexCoords(unsigned int packed, float* texcoord_out /*ecx*/)
		//{
		//	const static uint32_t func_addr = 0x5E4D30;
		//	__asm
		//	{
		//		mov		ecx, texcoord_out;
		//		push	packed;
		//		call	func_addr;
		//		add		esp, 4;
		//	}
		//}

		//std::int16_t G_ModelIndex(const char* model_name /*edi*/)
		//{
		//	const static uint32_t G_ModelIndex_func = 0x54A480;
		//	__asm
		//	{
		//		mov		edi, model_name;
		//		call	G_ModelIndex_func;
		//	}
		//}

		//bool DB_FileExists(const char* file_name, game::DB_FILE_EXISTS_PATH src)
		//{
		//	const static uint32_t DB_FileExists_func = 0x48FC10;
		//	__asm
		//	{
		//		push	src;
		//		mov		eax, file_name;

		//		call	DB_FileExists_func;
		//		add     esp, 4h;
		//	}
		//}
	} // sp-end

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	namespace mp
	{
	} // mp-end

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	/*GfxCmdBufSourceState* get_cmdbufsourcestate()
	{
		const auto out = reinterpret_cast<GfxCmdBufSourceState*>(game::is_mp ? *game::mp::gfxCmdBufSourceState_ptr : *game::sp::gfxCmdBufSourceState_ptr);
		return out;
	}

	GfxBackEndData* get_frontenddata()
	{
		const auto out = reinterpret_cast<GfxBackEndData*>(game::is_mp ? NULL : *sp::frontEndDataOut_ptr);
		return out;
	}

	GfxBackEndData* get_backenddata()
	{
		const auto out = reinterpret_cast<GfxBackEndData*>(game::is_mp ? *mp::backEndDataOut_ptr : *sp::backEndDataOut_ptr);
		return out;
	}*/

	void Cmd_AddCommand(const char* name, void(*callback)(), cmd_function_s* data, char)
	{
		data->name = name;
		data->function = callback;
		data->next = game::is_sp ? *sp::cmd_ptr : nullptr;
		data->consoleAccess = true;

		if (game::is_sp)
		{
			*game::sp::cmd_ptr = data;
		}
		else
		{
			__debugbreak(); // TODO
		}
	}

	// *
	// mp-sp functions

	/*utils::function<dvar_s* (const char* dvarName, int type, int flags, DvarValue value, DvarLimits domain, const char* description)> Dvar_RegisterVariant;

	dvar_s* Dvar_RegisterFloat(const char* name, float value, float min, float max, game::dvar_flags flags, const char* description)
	{
		game::DvarValue val = {};
		val.value = value;

		game::DvarLimits lim = {};
		lim.value.min = min;
		lim.value.max = max;

		return game::Dvar_RegisterVariant(name, game::dvar_type::value, flags, val, lim, description);
	}

	void init_offsets()
	{
		Dvar_RegisterVariant = SELECT(0x5C5100, 0x5EED90);
	}*/
}
