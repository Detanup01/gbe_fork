// My own modified version of ColdClientLoader originally written by Rat431

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>

bool IsNotRelativePathOrRemoveFileName(WCHAR* output, bool Remove)
{
	int LG = lstrlenW(output);
	for (int i = LG; i > 0; i--) {
		if (output[i] == '\\') {
			if(Remove)
				RtlFillMemory(&output[i], (LG - i) * sizeof(WCHAR), NULL);
			return true;
		}
	}
	return false;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	WCHAR CurrentDirectory[MAX_PATH] = { 0 };
	WCHAR Client64Path[MAX_PATH] = { 0 };
	WCHAR ClientPath[MAX_PATH] = { 0 };
	WCHAR ExeFile[MAX_PATH] = { 0 };
	WCHAR ExeRunDir[MAX_PATH] = { 0 };
	WCHAR ExeCommandLine[4096] = { 0 };
	WCHAR AppId[128] = { 0 };

	STARTUPINFOW info = { 0 };
	SecureZeroMemory(&info, sizeof(info));
	info.cb = sizeof(info);

	PROCESS_INFORMATION processInfo = { 0 };
	SecureZeroMemory(&processInfo, sizeof(processInfo));

	int Length = GetModuleFileNameW(GetModuleHandleW(NULL), CurrentDirectory, sizeof(CurrentDirectory)) + 1;
	for (int i = Length; i > 0; i--) {
		if (CurrentDirectory[i] == '\\') {
			lstrcpyW(&CurrentDirectory[i + 1], L"ColdClientLoader.ini");
			break;
		}
	}
	if (GetFileAttributesW(CurrentDirectory) == INVALID_FILE_ATTRIBUTES) {
		MessageBoxA(NULL, "Couldn't find the configuration file(ColdClientLoader.ini).", "ColdClientLoader", MB_ICONERROR);
		return 0;
	}

	GetPrivateProfileStringW(L"SteamClient", L"SteamClient64Dll", L"", Client64Path, MAX_PATH, CurrentDirectory);
	GetPrivateProfileStringW(L"SteamClient", L"SteamClientDll", L"", ClientPath, MAX_PATH, CurrentDirectory);
	GetPrivateProfileStringW(L"SteamClient", L"Exe", NULL, ExeFile, MAX_PATH, CurrentDirectory);
	GetPrivateProfileStringW(L"SteamClient", L"ExeRunDir", NULL, ExeRunDir, MAX_PATH, CurrentDirectory);
	GetPrivateProfileStringW(L"SteamClient", L"ExeCommandLine", NULL, ExeCommandLine, 4096, CurrentDirectory);
	GetPrivateProfileStringW(L"SteamClient", L"AppId", NULL, AppId, sizeof(AppId), CurrentDirectory);

	if (AppId[0]) {
		SetEnvironmentVariableW(L"SteamAppId", AppId);
		SetEnvironmentVariableW(L"SteamGameId", AppId);
		SetEnvironmentVariableW(L"SteamOverlayGameId", AppId);
	} else {
		MessageBoxA(NULL, "You forgot to set the AppId.", "ColdClientLoader", MB_ICONERROR);
		return 0;
	}

	WCHAR TMP[MAX_PATH] = {};
	if (!IsNotRelativePathOrRemoveFileName(Client64Path, false)) {
		lstrcpyW(TMP, Client64Path);
		SecureZeroMemory(Client64Path, sizeof(Client64Path));
		GetFullPathNameW(TMP, MAX_PATH, Client64Path, NULL);
	}
	if (!IsNotRelativePathOrRemoveFileName(ClientPath, false)) {
		lstrcpyW(TMP, ClientPath);
		SecureZeroMemory(ClientPath, sizeof(ClientPath));
		GetFullPathNameW(TMP, MAX_PATH, ClientPath, NULL);
	}
	if (!IsNotRelativePathOrRemoveFileName(ExeFile, false)) {
		lstrcpyW(TMP, ExeFile);
		SecureZeroMemory(ExeFile, sizeof(ExeFile));
		GetFullPathNameW(TMP, MAX_PATH, ExeFile, NULL);
	}
	if (!IsNotRelativePathOrRemoveFileName(ExeRunDir, false)) {
		lstrcpyW(TMP, ExeRunDir);
		SecureZeroMemory(ExeRunDir, sizeof(ExeRunDir));
		GetFullPathNameW(TMP, MAX_PATH, ExeRunDir, NULL);
	}

	if (GetFileAttributesW(Client64Path) == INVALID_FILE_ATTRIBUTES) {
		MessageBoxA(NULL, "Couldn't find the requested SteamClient64Dll.", "ColdClientLoader", MB_ICONERROR);
		return 0;
	}

	if (GetFileAttributesW(ClientPath) == INVALID_FILE_ATTRIBUTES) {
		MessageBoxA(NULL, "Couldn't find the requested SteamClientDll.", "ColdClientLoader", MB_ICONERROR);
		return 0;
	}

	if (GetFileAttributesW(ExeFile) == INVALID_FILE_ATTRIBUTES) {
		MessageBoxA(NULL, "Couldn't find the requested Exe file.", "ColdClientLoader", MB_ICONERROR);
		return 0;
	}

	WCHAR CommandLine[16384] = { 0 };
	_snwprintf(CommandLine, _countof(CommandLine), L"\"%ls\" %ls %ls", ExeFile, ExeCommandLine, lpCmdLine);
	if (!ExeFile[0] || !CreateProcessW(ExeFile, CommandLine, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, ExeRunDir, &info, &processInfo))
	{
		MessageBoxA(NULL, "Unable to load the requested EXE file.", "ColdClientLoader", MB_ICONERROR);
		return 0;
	}
	HKEY Registrykey;
	// Declare some variables to be used for Steam registry.
	DWORD UserId = 0x03100004771F810D & 0xffffffff;
	DWORD ProcessID = GetCurrentProcessId();

	bool orig_steam = false;
	DWORD keyType = REG_SZ;
	WCHAR OrgSteamCDir[MAX_PATH] = { 0 };
	WCHAR OrgSteamCDir64[MAX_PATH] = { 0 };
	DWORD Size1 = MAX_PATH;
	DWORD Size2 = MAX_PATH;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS)
	{
		orig_steam = true;
		// Get original values to restore later.
		RegQueryValueExW(Registrykey, L"SteamClientDll", 0, &keyType, (LPBYTE)& OrgSteamCDir, &Size1);
		RegQueryValueExW(Registrykey, L"SteamClientDll64", 0, &keyType, (LPBYTE)& OrgSteamCDir64, &Size2);
	} else {
		if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, 0, REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, NULL, &Registrykey, NULL) != ERROR_SUCCESS)
		{
			MessageBoxA(NULL, "Unable to patch Steam process informations on the Windows registry.", "ColdClientLoader", MB_ICONERROR);
			TerminateProcess(processInfo.hProcess, NULL);
			return 0;
		}
	}

	// Set values to Windows registry.
	RegSetValueExA(Registrykey, "ActiveUser", NULL, REG_DWORD, (LPBYTE)& UserId, sizeof(DWORD));
	RegSetValueExA(Registrykey, "pid", NULL, REG_DWORD, (LPBYTE)& ProcessID, sizeof(DWORD));

	{
		// Before saving to the registry check again if the path was valid and if the file exist
		if (GetFileAttributesW(ClientPath) != INVALID_FILE_ATTRIBUTES) {
			RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (LPBYTE)ClientPath, (DWORD)(lstrlenW(ClientPath) * sizeof(WCHAR)) + 1);
		}
		else {
			RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (LPBYTE)"", 0);
		}
		if (GetFileAttributesW(Client64Path) != INVALID_FILE_ATTRIBUTES) {
			RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (LPBYTE)Client64Path, (DWORD)(lstrlenW(Client64Path) * sizeof(WCHAR)) + 1);
		}
		else {
			RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (LPBYTE)"", 0);
		}
	}
	RegSetValueExA(Registrykey, "Universe", NULL, REG_SZ, (LPBYTE)"Public", (DWORD)lstrlenA("Public") + 1);

	// Close the HKEY Handle.
	RegCloseKey(Registrykey);

	ResumeThread(processInfo.hThread);
	WaitForSingleObject(processInfo.hThread, INFINITE);
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	if (orig_steam) {
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS)
		{
			// Restore the values.
			RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (LPBYTE)OrgSteamCDir, Size1);
			RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (LPBYTE)OrgSteamCDir64, Size2);

			// Close the HKEY Handle.
			RegCloseKey(Registrykey);
		}
	}

	return 0;
}
