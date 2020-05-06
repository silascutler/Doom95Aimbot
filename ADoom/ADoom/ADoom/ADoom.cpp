#include "stdafx.h"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <tchar.h>
#include <Windows.h>

#include "ADoom.h"

ADoom::ADoom(DWORD CPID)
{
	DH = OpenProcess(PROCESS_ALL_ACCESS, FALSE, CPID);

	printf("Opened: %d\n", CPID);

	if (DH != INVALID_HANDLE_VALUE)
	{
		return;
	}

	throw std::runtime_error("Can't open process!");


}

ADoom::~ADoom() {
	CloseHandle(DH);
}

template<typename ReadType>
ReadType ADoom::rM(DWORD RAddress, DWORD Offset)
{
	ReadType Result;
	PVOID	 External = reinterpret_cast<PVOID>(RAddress + Offset);

	ReadProcessMemory(DH, External, &Result, sizeof(Result), NULL);
	return Result;
}

VOID ADoom::sT(DWORD FPtr)
{
	HANDLE RT;

	RT = CreateRemoteThread(DH, NULL, 0, (LPTHREAD_START_ROUTINE)FPtr,
		NULL, 0, NULL);

	if (RT != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(RT, INFINITE);
	}
}
DWORD ADoom::aM(SIZE_T Size)
{
	LPVOID RAddress = VirtualAllocEx(DH, NULL, Size, MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);
	DWORD  Cast = reinterpret_cast<DWORD>(RAddress);

	return Cast;
}


BOOL ADoom::wM(DWORD RAddress, PVOID LAddress, SIZE_T Size)
{
	BOOL	Status = FALSE;
	PVOID	External = reinterpret_cast<PVOID>(RAddress);

	if (WriteProcessMemory(DH, External, LAddress, Size, NULL))
	{
		Status = TRUE;
	}

	return Status;
}



int main()
{
	HWND	DoomWindow;
	DWORD	PID;

	DoomWindow = FindWindow(NULL, _T("Doom 95"));
	if (!DoomWindow) goto out;
	GetWindowThreadProcessId(DoomWindow, &PID);

	printf("PID: %d\n", PID);

	try
	{
		ADoom	DAim(PID);
		BYTE	Patch[2] = { 0xEB, 0x33 };
		DWORD	PAddress = 0x42264F;
		BYTE	Payload[] = { 0xB8, 0x18, 0x25, 0x48, 0x00, 0x8B, 0x00,
			0xBF, 0x70, 0xF6, 0x41, 0x00, 0xFF, 0xD7,
			0xC3 };
		DWORD	Location, PSize = sizeof(Payload);
		DWORD	PPlayer = 0x482518, Player, Target;
		int		THealth;

		/*
		Patch:
		An unconditional JMP instruction that allows Player->target
		to be updated on every attack.
		*/
		printf("Applying Patch @ %lX\n", PAddress);
		DAim.wM(PAddress, Patch, sizeof(Patch));

		printf("Allocating Memory(%d)\n", PSize);
		Location = DAim.aM(PSize);

		printf("Storing Function @ %lX\n", Location);
		DAim.wM(Location, Payload, PSize);

		puts("[0x00sec] Aimbot starting.");

		while (TRUE)
		{
			Player = DAim.rM<DWORD>(PPlayer, 0);
			Target = DAim.rM<DWORD>(Player, 0x78);

			// Did any enemy attack us?
			if (Target != 0)
			{
				// If yes, is it still alive?
				THealth = DAim.rM<int>(Target, 0x6C);
				if (THealth > 0)
				{
					// Aim at it.
					DAim.sT(Location);
				}
			}
		}
	}
	catch (const std::runtime_error &err) {}


	while (1)
	{
		Sleep(1);
	}
out:
	return 0;
}
