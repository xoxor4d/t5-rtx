#pragma once
#include "utils/function.hpp"

#define SELECT(mp_addr, sp_addr) (game::is_mp ? mp_addr : sp_addr)

namespace game
{
	extern bool is_sp;
	extern bool is_mp;

	extern std::vector<std::string> loaded_modules;
	extern HWND main_window;
	extern bool is_game_mod;

	static inline float COLOR_WHITE[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static inline float COLOR_BLACK[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	namespace sp
	{
		extern DxGlobals* dx;
		//extern clipMap_t* cm;
		extern r_global_permanent_t* rgp;
		extern GfxCmdBufSourceState* gfxCmdBufSourceState;
		extern GfxCmdBufState* gfxCmdBufState;
		extern Ui3dTextureWindow* g_ui3d_windows;

		static DWORD* g_world_draw_ptr = (DWORD*)(0x460C0B0);
		extern GfxWorldDraw* get_g_world_draw();

		static DWORD* frontenddata_out_ptr = (DWORD*)(0x3B3708C);
		extern game::GfxBackEndData* get_frontenddata_out();

		//extern GfxWorldDraw* g_worldDraw;
		extern GfxBuffers* gfx_buf;

		extern cmd_function_s** cmd_ptr;

		extern CmdArgs* cmd_args();
		extern int cmd_argc();
		extern const char* cmd_argv(int arg_index);

		//static DWORD* gfxCmdBufSourceState_ptr = (DWORD*)(0x829EA8);
		//static DWORD* frontEndDataOut_ptr = (DWORD*)(0x3DCB498);
		//static DWORD* backEndDataOut_ptr = (DWORD*)(0x3DCB4CC);

		static utils::function<dvar_s* __cdecl (const char* name)> Dvar_FindVar = 0x5AE810;
		static utils::function<dvar_s* __cdecl (const char* name, const char** data, int value, dvar_flags flags, const char* desc)> Dvar_RegisterEnum = 0x51BD00;
		static utils::function<dvar_s* __cdecl (const char* name, bool value, dvar_flags flags, const char* desc)> Dvar_RegisterBool = 0x45BB20;
		static utils::function<dvar_s* __cdecl (const char* name, float value, float min, float max, dvar_flags flags, const char* desc)> Dvar_RegisterFloat = 0x679020;

		// 61BE00
		static utils::function<Material* __cdecl (const char* name)> Material_RegisterHandle = 0x6D4080;
		static utils::function<BOOL __cdecl (game::dvar_s*)> Dvar_HasLatchedValue = 0x6D4080;
		static utils::function<void __cdecl (game::dvar_s*)> Dvar_MakeLatchedCurrent = 0x61CFC0;

		//static utils::function<void __fastcall (int, GfxCmdBufSourceState*)> R_Set3D = 0x7244C0;

		static utils::function<void* (int)> Sys_GetValue = 0x67D4F0;
		static utils::function<BOOL ()> Sys_IsMainThread = 0x5A48F0;

		typedef void(*Cmd_ExecuteSingleCommand_t)(int controller, int a2, const char* cmd);
			extern Cmd_ExecuteSingleCommand_t Cmd_ExecuteSingleCommand;

		typedef void(*Cbuf_AddText_t)(int controller, const char* text);
			extern Cbuf_AddText_t Cbuf_AddText;

		typedef void(*Com_PrintMessage_t)(int, const char*, char);
			extern Com_PrintMessage_t Com_PrintMessage;

		//typedef void(*Com_Error_t)(int type, const char* message, ...);
		//	extern Com_Error_t Com_Error;

		//typedef void(*DB_EnumXAssets_FastFile_t)(XAssetType type, void(*)(XAssetHeader, void*), void* userdata, bool overrides);
		//	extern DB_EnumXAssets_FastFile_t DB_EnumXAssets_FastFile;

		//typedef void(*DB_LoadXAssets_t)(XZoneInfo* zoneInfo, unsigned int zone_count, int sync);
		//	extern DB_LoadXAssets_t DB_LoadXAssets;

		//extern scr_const_t* scr_const;

		//void Cbuf_AddText(const char* text /*eax*/);
		//void Vec2UnpackTexCoords(unsigned int packed, float* texcoord_out /*ecx*/);

		static utils::function<void __cdecl (unsigned int packed, float* out)> Vec2UnpackTexCoords = 0x5AC8E0;

		//std::int16_t G_ModelIndex(const char* model_name /*eax*/); // ASM
		//extern bool DB_FileExists(const char* file_name, game::DB_FILE_EXISTS_PATH);
		static utils::function<bool __cdecl (const char* name, game::DB_FILE_EXISTS_PATH src)> DB_FileExists = 0x5B4B50;


		//static utils::function<game::gentity_s* ()> G_Spawn = 0x54EAB0;
		//static utils::function<bool(game::gentity_s*)> G_CallSpawnEntity = 0x5465A0;
		//static utils::function<void(entityState_s*)> G_DObjUpdate = 0x54AC40;

		void R_SetRenderTargetSize(GfxCmdBufSourceState* source /*ecx*/, std::uint8_t rt_index /*al*/); // asm
		static utils::function<void(GfxCmdBufSourceState* source, GfxCmdBufState* state, std::uint8_t rt_index)> R_SetRenderTarget = 0x726650;

		void R_AddCellSurfacesAndCullGroupsInFrustumDelayed(GfxCell* cell /*eax*/, DpvsPlane* planes /*edi*/, int planeCount, int frustumPlaneCount); // asm
		void R_AddDebugString(DebugGlobals* debug_globals_entry /*edi*/, const float* origin, const float* color, float scale, const char* string); // asm
	}

	// ------------------------------------------------------------------------------------------------------------
	// ############################################################################################################
	// ------------------------------------------------------------------------------------------------------------

	namespace mp
	{
	}

	//extern GfxCmdBufSourceState* get_cmdbufsourcestate();
	//extern GfxBackEndData* get_frontenddata();
	//extern GfxBackEndData* get_backenddata();

	void Cmd_AddCommand(const char* name, void(*callback)(), cmd_function_s* data, char);
	//dvar_s* Dvar_RegisterFloat(const char* name, float value, float min, float max, game::dvar_flags flags, const char* description);

	// mp-sp functions
	//extern utils::function<dvar_s* (const char*, int, int, DvarValue, DvarLimits, const char*)> Dvar_RegisterVariant;
	//void init_offsets();
}
