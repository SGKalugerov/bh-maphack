#include <Windows.h>
#include "D2Ptrs.h"

DWORD __declspec(naked) __fastcall D2CLIENT_GetUnitName_STUB(DWORD UnitAny)
{
	__asm
	{
		mov eax, ecx
		jmp D2CLIENT_GetUnitName_I
	}
}

DWORD __declspec(naked) __fastcall D2CLIENT_InitAutomapLayer(DWORD nLayerNo)
{
	__asm 
	{
		push eax;
		mov eax, ecx;
		call D2CLIENT_InitAutomapLayer_I;
		pop eax;
		ret;
	}
}

DWORD __declspec(naked) __fastcall TestPvpFlag_STUB(DWORD planum1, DWORD planum2, DWORD flagmask)
{
	__asm 
	{
		push esi;
		push [esp+8];
		mov esi, edx;
		mov edx, ecx;
		call D2CLIENT_TestPvpFlag_I;
		pop esi;
		ret 4;
	}
}

DWORD __declspec(naked) __fastcall D2CLIENT_GetLevelName_STUB(DWORD levelId)
{
	__asm
	{
		mov esi, ecx
		jmp D2CLIENT_GetLevelName_I
	}
}

DWORD __declspec(naked) __fastcall D2CLIENT_GetUIVar_STUB(DWORD varno)
{
	__asm 
	{
		mov eax, ecx;
		jmp D2CLIENT_GetUiVar_I;
	}
}

__declspec(naked) CellFile* __fastcall D2CLIENT_LoadUiImage(CHAR* szPath)
{
	__asm
	{
		MOV EAX, ECX
		PUSH 0
		CALL D2CLIENT_LoadUiImage_I
		RETN
	}
}

DWORD __declspec(naked) __fastcall D2CLIENT_ClickParty_ASM(RosterUnit* RosterUnit, DWORD Mode)
{
	__asm
	{
		mov eax, ecx
		jmp D2CLIENT_ClickParty_I
	}
}

void __declspec(naked) __fastcall D2CLIENT_PlaySound(int SoundNo)
{
	__asm
	{
		push ebx
		mov ebx, ecx
		push 0
		push 0
		push 0
		push 0
		call D2CLIENT_PlaySound_I
		pop ebx;
		ret
	}
}

__declspec(naked) void* __stdcall D2WIN_LoadMpq(DWORD OrderFlag, const char* szDllFile, const char* szMpqFile, const char* szName, DWORD _1, BOOL(__stdcall* fnMPQMissing)())
{
	__asm
	{
		push[esp + 0x18]
		push[esp + 0x18]
		push[esp + 0x18]
		push[esp + 0x18]
		push[esp + 0x18]
		mov eax, [esp + 0x18]
		call D2WIN_LoadMpq_I
		ret 0x18
	}
}