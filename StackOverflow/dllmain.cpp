#define WIN32_LEAN_AND_MEAN

#include <Windows.h>


DWORD WINAPI ThreadFunction(LPVOID lpParam) {
	if (lpParam != NULL) {
		return 1;
	}

	return ThreadFunction(lpParam);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		CreateThread(NULL, 0, ThreadFunction, NULL, 0, NULL);
	}

	return TRUE;
}

