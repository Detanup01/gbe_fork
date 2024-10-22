#NoTrayIcon

#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Outfile_type=a3x
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****

#include <File.au3>

; ARC_NAME

$arc_lobby_connect = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "ARC_NAME", "lobby_connect", "lobby_connect.7z")

; EXE_PATH

$lobby_connect_release = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "EXE_PATH", "lobby_connect_release", "lobby_connect.exe")
$lobby_connect64_release = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "EXE_PATH", "lobby_connect64_release", "lobby_connect64.exe")

$lobby_connect_7z = @ScriptDir & '\steam_misc\tools\lobby_connect\' & $arc_lobby_connect
$lobby_connect_dst = @ScriptDir & '\steam_misc\tools\lobby_connect'
$lobby_connect_exe = ''
Switch @OSArch
	Case 'X64'
		$lobby_connect_exe = StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1)) ; lobby_connect64.exe
		If Not FileExists(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1))) Then
			ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & $lobby_connect_7z & '" -o"' & $lobby_connect_dst & '" -aoa', "", "", @SW_HIDE)
		EndIf
	Case 'X86'
		$lobby_connect_exe = StringTrimLeft($lobby_connect_release, StringInStr($lobby_connect_release, "\", 0, -1)) ; lobby_connect.exe
		If Not FileExists(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect_release, StringInStr($lobby_connect_release, "\", 0, -1))) Then
			ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & $lobby_connect_7z & '" -o"' & $lobby_connect_dst & '" -aoa', "", "", @SW_HIDE)
		EndIf
	Case Else
		$lobby_connect_exe = StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1)) ; lobby_connect64.exe
		If Not FileExists(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1))) Then
			ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & $lobby_connect_7z & '" -o"' & $lobby_connect_dst & '" -aoa', "", "", @SW_HIDE)
		EndIf
EndSwitch

If Not FileExists(@ScriptDir & '\steam_api.dll') Then FileDelete(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect_release, StringInStr($lobby_connect_release, "\", 0, -1)))
If Not FileExists(@ScriptDir & '\steam_api64.dll') Then FileDelete(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1)))

Switch @OSArch
	Case 'X64'
		If Not FileExists(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1))) Then
			$lobby_connect_exe = StringTrimLeft($lobby_connect_release, StringInStr($lobby_connect_release, "\", 0, -1))
		EndIf
	Case 'X86'
		If Not FileExists(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect_release, StringInStr($lobby_connect_release, "\", 0, -1))) Then
			$lobby_connect_exe = StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1))
		EndIf
	Case Else
		If Not FileExists(@ScriptDir & '\steam_misc\tools\lobby_connect\' & StringTrimLeft($lobby_connect64_release, StringInStr($lobby_connect64_release, "\", 0, -1))) Then
			$lobby_connect_exe = StringTrimLeft($lobby_connect_release, StringInStr($lobby_connect_release, "\", 0, -1))
		EndIf
EndSwitch

RunWait($lobby_connect_dst & '\' & $lobby_connect_exe, @ScriptDir, @SW_SHOW)