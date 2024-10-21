#NoTrayIcon

#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Outfile_type=a3x
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****

; ARC_NAME

$arc_steam_api = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "ARC_NAME", "steam_api", "steam_api.7z")
$arc_steam_api64 = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "ARC_NAME", "steam_api64", "steam_api64.7z")

$arc_steamclient = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "ARC_NAME", "steamclient", "steamclient.7z")
$arc_steamclient64 = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "ARC_NAME", "steamclient64", "steamclient64.7z")

; DLL_PATH

$steam_api_release = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steam_api_release", "release\steam_api.dll")
$steam_api64_release = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steam_api64_release", "release\steam_api64.dll")

$steam_api_debug = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steam_api_debug", "debug\steam_api.dll")
$steam_api64_debug = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steam_api64_debug", "debug\steam_api64.dll")

$steamclient_release = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steamclient_release", "release\steamclient.dll")
$steamclient64_release = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steamclient64_release", "release\steamclient64.dll")

$steamclient_debug = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steamclient_debug", "debug\steamclient.dll")
$steamclient64_debug = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "DLL_PATH", "steamclient64_debug", "debug\steamclient64.dll")

If FileReadLine(@ScriptDir & "\steam_settings\emu_version.txt", 1) == "release" Then
	If FileExists(@ScriptDir & "\" & $arc_steam_api) Then ; replace steam_api.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steam_api & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api, 3) & '" -aoa', "", "", @SW_HIDE)
		$steam_api_dst = StringTrimLeft($steam_api_release, StringInStr($steam_api_release, "\", 0, -1))
		$steam_api_debug_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api, 3) & "\" & $steam_api_debug, "\\", "\")
		If FileExists($steam_api_dst) Then
			FileMove($steam_api_debug_src, $steam_api_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "debug" & @CRLF & @CRLF & "you are currently using the 'debug' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'release' version")
		FileClose($hFileOpen)
	EndIf
	If FileExists(@ScriptDir & "\" & $arc_steam_api64) Then ; replace steam_api64.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steam_api64 & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api64, 3) & '" -aoa', "", "", @SW_HIDE)
		$steam_api64_dst = StringTrimLeft($steam_api64_release, StringInStr($steam_api64_release, "\", 0, -1))
		$steam_api64_debug_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api64, 3) & "\" & $steam_api64_debug, "\\", "\")
		If FileExists($steam_api64_dst) Then
			FileMove($steam_api64_debug_src, $steam_api64_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api64, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "debug" & @CRLF & @CRLF & "you are currently using the 'debug' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'release' version")
		FileClose($hFileOpen)
	EndIf
	If FileExists(@ScriptDir & "\" & $arc_steamclient) Then ; replace steamclient.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steamclient & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient, 3) & '" -aoa', "", "", @SW_HIDE)
		$steamclient_dst = StringTrimLeft($steamclient_release, StringInStr($steamclient_release, "\", 0, -1))
		$steamclient_debug_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient, 3) & "\" & $steamclient_debug, "\\", "\")
		If FileExists($steamclient_dst) Then
			FileMove($steamclient_debug_src, $steamclient_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "debug" & @CRLF & @CRLF & "you are currently using the 'debug' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'release' version")
		FileClose($hFileOpen)
	EndIf
	If FileExists(@ScriptDir & "\" & $arc_steamclient64) Then ; replace steamclient64.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steamclient64 & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient64, 3) & '" -aoa', "", "", @SW_HIDE)
		$steamclient64_dst = StringTrimLeft($steamclient64_release, StringInStr($steamclient64_release, "\", 0, -1))
		$steamclient64_debug_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient64, 3) & "\" & $steamclient64_debug, "\\", "\")
		If FileExists($steamclient64_dst) Then
			FileMove($steamclient64_debug_src, $steamclient64_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient64, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "debug" & @CRLF & @CRLF & "you are currently using the 'debug' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'release' version")
		FileClose($hFileOpen)
	EndIf
ElseIf FileReadLine(@ScriptDir & "\steam_settings\emu_version.txt", 1) == "debug" Then
	If FileExists(@ScriptDir & "\" & $arc_steam_api) Then ; replace steam_api.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steam_api & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api, 3) & '" -aoa', "", "", @SW_HIDE)
		$steam_api_dst = StringTrimLeft($steam_api_debug, StringInStr($steam_api_debug, "\", 0, -1))
		$steam_api_release_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api, 3) & "\" & $steam_api_release, "\\", "\")
		If FileExists($steam_api_dst) Then
			FileMove($steam_api_release_src, $steam_api_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "release" & @CRLF & @CRLF & "you are currently using the 'release' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'debug' version")
		FileClose($hFileOpen)
	EndIf
	If FileExists(@ScriptDir & "\" & $arc_steam_api64) Then ; replace steam_api.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steam_api64 & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api64, 3) & '" -aoa', "", "", @SW_HIDE)
		$steam_api64_dst = StringTrimLeft($steam_api64_debug, StringInStr($steam_api64_debug, "\", 0, -1))
		$steam_api64_release_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api64, 3) & "\" & $steam_api64_release, "\\", "\")
		If FileExists($steam_api64_dst) Then
			FileMove($steam_api64_release_src, $steam_api64_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steam_api64, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "release" & @CRLF & @CRLF & "you are currently using the 'release' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'debug' version")
		FileClose($hFileOpen)
	EndIf
	If FileExists(@ScriptDir & "\" & $arc_steamclient) Then ; replace steamclient.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steamclient & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient, 3) & '" -aoa', "", "", @SW_HIDE)
		$steamclient_dst = StringTrimLeft($steamclient_debug, StringInStr($steamclient_debug, "\", 0, -1))
		$steamclient_release_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient, 3) & "\" & $steamclient_release, "\\", "\")
		If FileExists($steamclient_dst) Then
			FileMove($steamclient_release_src, $steamclient_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "release" & @CRLF & @CRLF & "you are currently using the 'release' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'debug' version")
		FileClose($hFileOpen)
	EndIf
	If FileExists(@ScriptDir & "\" & $arc_steamclient64) Then ; replace steamclient64.dll if it exists
		$7za_exit = ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & @ScriptDir & "\" & $arc_steamclient64 & '" -o"' & @ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient64, 3) & '" -aoa', "", "", @SW_HIDE)
		$steamclient64_dst = StringTrimLeft($steamclient64_debug, StringInStr($steamclient64_debug, "\", 0, -1))
		$steamclient64_release_src = StringReplace(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient64, 3) & "\" & $steamclient64_release, "\\", "\")
		If FileExists($steamclient64_dst) Then
			FileMove($steamclient64_release_src, $steamclient64_dst, 1)
		EndIf
		DirRemove(@ScriptDir & "\steam_misc\" & StringTrimRight($arc_steamclient64, 3), 1)
		$hFileOpen = FileOpen(@ScriptDir & "\steam_settings\emu_version.txt", 2+8)
		FileWrite($hFileOpen, "release" & @CRLF & @CRLF & "you are currently using the 'release' version of the emulator" & @CRLF & "run 'gse_debug_switch.exe' if you want to use the 'debug' version")
		FileClose($hFileOpen)
	EndIf
EndIf


