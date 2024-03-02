#include "std_include.hpp"

// Do necessary assertions here
// Some compilers treat them differently which causes a size mismatch

// WinAPI types
AssertSize(DWORD, 4);
AssertSize(WORD, 2);
AssertSize(BYTE, 1);

// 128 bit integers (only x64)
//AssertSize(__int128, 16);
//AssertSize(unsigned __int128, 16);

// 64 bit integers
AssertSize(__int64, 8);
AssertSize(unsigned __int64, 8);
AssertSize(long long, 8);
AssertSize(unsigned long long, 8);
AssertSize(int64_t, 8);
AssertSize(uint64_t, 8);
AssertSize(std::int64_t, 8);
AssertSize(std::uint64_t, 8);

// 64 bit double precision floating point numbers
AssertSize(double, 8);

// 32 bit integers
AssertSize(__int32, 4);
AssertSize(unsigned __int32, 4);
AssertSize(int, 4);
AssertSize(unsigned int, 4);
AssertSize(long, 4);
AssertSize(unsigned long, 4);
AssertSize(int32_t, 4);
AssertSize(uint32_t, 4);
AssertSize(std::int32_t, 4);
AssertSize(std::uint32_t, 4);

// 32 bit single precision floating point numbers
AssertSize(float, 4);

// 16 bit integers
AssertSize(__int16, 2);
AssertSize(unsigned __int16, 2);
AssertSize(short, 2);
AssertSize(unsigned short, 2);
AssertSize(int16_t, 2);
AssertSize(uint16_t, 2);
AssertSize(std::int16_t, 2);
AssertSize(std::uint16_t, 2);

// 8 bit integers
AssertSize(bool, 1);
AssertSize(__int8, 1);
AssertSize(unsigned __int8, 1);
AssertSize(char, 1);
AssertSize(unsigned char, 1);
AssertSize(int8_t, 1);
AssertSize(uint8_t, 1);
AssertSize(std::int8_t, 1);
AssertSize(std::uint8_t, 1);

// Ensure pointers are 4 bytes in size (32-bit)
static_assert(sizeof(intptr_t) == 4 && sizeof(void*) == 4 && sizeof(size_t) == 4, "This doesn't seem to be a 32-bit environment!");

// Disable telemetry data logging
//extern "C"
//{
//	void _cdecl __vcrt_initialize_telemetry_provider() {}
//	void _cdecl __telemetry_main_invoke_trigger() {}
//	void _cdecl __telemetry_main_return_trigger() {}
//	void _cdecl __vcrt_uninitialize_telemetry_provider() {}
//
//	// Enable 'High Performance Graphics'
//	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
//};

// Bridge d3d9
#pragma comment(linker, "/export:D3DXCompileShader=d3dx9_43.D3DXCompileShader")
#pragma comment(linker, "/export:D3DXSaveSurfaceToFileA=d3dx9_43.D3DXSaveSurfaceToFileA")
#pragma comment(linker, "/export:D3DXCreateBuffer=d3dx9_43.D3DXCreateBuffer")
#pragma comment(linker, "/export:D3DXGetShaderConstantTable=d3dx9_43.D3DXGetShaderConstantTable")
#pragma comment(linker, "/export:D3DXGetShaderInputSemantics=d3dx9_43.D3DXGetShaderInputSemantics")
#pragma comment(linker, "/export:D3DXGetShaderOutputSemantics=d3dx9_43.D3DXGetShaderOutputSemantics")