#ifndef MUENDEFS_H_INCLUDED
#define MUENDEFS_H_INCLUDED

#ifdef _WIN32
#    ifdef WRMUENAM_BUILD
#        define WRMUENAM_DLL_API __declspec(dllexport)
#    else
#        define WRMUENAM_DLL_API __declspec(dllimport)
#    endif
#	define WRMUENAM_CDECL __cdecl
#	define WRMUENAM_STDCALL __stdcall
#else
#   define WRMUENAM_DLL_API
#	define WRMUENAM_CDECL
#	define WRMUENAM_STDCALL
#endif

#ifdef _WIN32
	#include <windows.h>
	#include <tchar.h>
	#define MUEN_DEFO_INIREGDIR "C:\\ProgramData\\muengine"
#endif

#ifdef __linux__
	#define MUEN_DEFO_INIREGDIR "/usr/share/muengine"
#endif

#ifdef __APPLE__
	#define MUEN_DEFO_INIREGDIR "/Applications/muengine"
#endif

#endif
