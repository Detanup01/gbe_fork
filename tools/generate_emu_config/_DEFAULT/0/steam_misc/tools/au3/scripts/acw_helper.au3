#NoTrayIcon

#Region ;**** Directives created by AutoIt3Wrapper_GUI ****
#AutoIt3Wrapper_Outfile_type=a3x
#EndRegion ;**** Directives created by AutoIt3Wrapper_GUI ****

#include <File.au3>
#include <Array.au3>

; ARC_NAME

$arc_extra_acw = IniRead(@ScriptDir & "\" & StringTrimRight(@ScriptName, 4) & ".ini", "ARC_NAME", "extra_acw", "extra_acw.zip")

$ach_watcher_arc = @ScriptDir & "\steam_misc\extra_acw\" & $arc_extra_acw
$ach_watcher_dst = @AppDataDir & "\Achievement Watcher"

If FileExists($ach_watcher_arc) Then

	$gse_saves = IniRead(@ScriptDir & "\steam_settings\configs.user.ini", "user::saves", "saves_folder_name", "GSE Saves")
	$local_save = IniRead(@ScriptDir & "\steam_settings\configs.user.ini", "user::saves", "local_save_path", "")
	$local_save = StringReplace($local_save, "./", "")
	$local_save = StringReplace($local_save, ".\", "")

	ShellExecuteWait(@ScriptDir & "\steam_misc\tools\7za\7za.exe", 'x "' & $ach_watcher_arc & '" -o"' & $ach_watcher_dst & '" -aoa', "", "", @SW_HIDE)

	If $local_save <> "" Then

		$userdir_line1 = '  {'
		$userdir_line2 = '    "path": "' & StringReplace(@ScriptDir & "\" & $gse_saves, "\", "\\") & '",'
		$userdir_line3 = '    "notify": true'
		$userdir_line4 = '  }'

		$file = @AppDataDir & "\Achievement Watcher\cfg\userdir.db"
		$temp = @AppDataDir & "\Achievement Watcher\cfg\userdir_temp.db"

		If Not FileExists($file) Then
			;FileCopy(@ScriptDir & "\steam_settings\ach\cfg\userdir.db", $file, 1)
			FileWriteLine($file, "[")
			FileWriteLine($file, "]")
			If FileExists($temp) Then FileDelete($temp)
			Local $aLines
			_FileReadToArray($file, $aLines)
			Local $lastline = _ArrayPop($aLines)
			_FileWriteFromArray($temp, $aLines)
			FileWriteLine($temp, $userdir_line1)
			FileWriteLine($temp, $userdir_line2)
			FileWriteLine($temp, $userdir_line3)
			FileWriteLine($temp, $userdir_line4)
			FileWriteLine($temp, $lastline)
			_ReplaceStringInFile($temp, $aLines[0] & @CRLF, "")
			FileMove($temp, $file, 1)
		Else
			Local $aLines, $hMatch
			_FileReadToArray($file, $aLines)
			For $i = 1 To $aLines[0]
				If StringInStr($aLines[$i], $userdir_line2) Then $hMatch = 1
			Next
			If $hMatch == 1 Then
				If FileExists($temp) Then FileDelete($temp)
			Else
				If FileExists($temp) Then FileDelete($temp)
				;Local $aLines
				;_FileReadToArray($file, $aLines)
				Local $lastline = _ArrayPop($aLines)
				_FileWriteFromArray($temp, $aLines)
				FileWriteLine($temp, $userdir_line1)
				FileWriteLine($temp, $userdir_line2)
				FileWriteLine($temp, $userdir_line3)
				FileWriteLine($temp, $userdir_line4)
				FileWriteLine($temp, $lastline)
				_ReplaceStringInFile($temp, $aLines[0] & @CRLF, "")
				FileMove($temp, $file, 1)
			EndIf
		EndIf

		_ReplaceStringInFile($file, "  }" & @CRLF & "  {", "  }," & @CRLF & "  {")

		; ---

		$userdir_line1 = '  {'
		$userdir_line2 = '    "path": "' & StringReplace(@ScriptDir & "\" & $local_save, "\", "\\") & '",'
		$userdir_line3 = '    "notify": true'
		$userdir_line4 = '  }'

		$file = @AppDataDir & "\Achievement Watcher\cfg\userdir.db"
		$temp = @AppDataDir & "\Achievement Watcher\cfg\userdir_temp.db"

		If Not FileExists($file) Then
			;FileCopy(@ScriptDir & "\steam_settings\ach\cfg\userdir.db", $file, 1)
			FileWriteLine($file, "[")
			FileWriteLine($file, "]")
			If FileExists($temp) Then FileDelete($temp)
			Local $aLines
			_FileReadToArray($file, $aLines)
			Local $lastline = _ArrayPop($aLines)
			_FileWriteFromArray($temp, $aLines)
			FileWriteLine($temp, $userdir_line1)
			FileWriteLine($temp, $userdir_line2)
			FileWriteLine($temp, $userdir_line3)
			FileWriteLine($temp, $userdir_line4)
			FileWriteLine($temp, $lastline)
			_ReplaceStringInFile($temp, $aLines[0] & @CRLF, "")
			FileMove($temp, $file, 1)
		Else
			Local $aLines, $hMatch
			_FileReadToArray($file, $aLines)
			For $i = 1 To $aLines[0]
				If StringInStr($aLines[$i], $userdir_line2) Then $hMatch = 1
			Next
			If $hMatch == 1 Then
				If FileExists($temp) Then FileDelete($temp)
			Else
				If FileExists($temp) Then FileDelete($temp)
				;Local $aLines
				;_FileReadToArray($file, $aLines)
				Local $lastline = _ArrayPop($aLines)
				_FileWriteFromArray($temp, $aLines)
				FileWriteLine($temp, $userdir_line1)
				FileWriteLine($temp, $userdir_line2)
				FileWriteLine($temp, $userdir_line3)
				FileWriteLine($temp, $userdir_line4)
				FileWriteLine($temp, $lastline)
				_ReplaceStringInFile($temp, $aLines[0] & @CRLF, "")
				FileMove($temp, $file, 1)
			EndIf
		EndIf

		_ReplaceStringInFile($file, "  }" & @CRLF & "  {", "  }," & @CRLF & "  {")

	Else

		$userdir_line1 = '  {'
		$userdir_line2 = '    "path": "' & StringReplace(@AppDataDir & "\" & $gse_saves, "\", "\\") & '",'
		$userdir_line3 = '    "notify": true'
		$userdir_line4 = '  }'

		$file = @AppDataDir & "\Achievement Watcher\cfg\userdir.db"
		$temp = @AppDataDir & "\Achievement Watcher\cfg\userdir_temp.db"

		If Not FileExists($file) Then
			;FileCopy(@ScriptDir & "\steam_settings\ach\cfg\userdir.db", $file, 1)
			FileWriteLine($file, "[")
			FileWriteLine($file, "]")
			If FileExists($temp) Then FileDelete($temp)
			Local $aLines
			_FileReadToArray($file, $aLines)
			Local $lastline = _ArrayPop($aLines)
			_FileWriteFromArray($temp, $aLines)
			FileWriteLine($temp, $userdir_line1)
			FileWriteLine($temp, $userdir_line2)
			FileWriteLine($temp, $userdir_line3)
			FileWriteLine($temp, $userdir_line4)
			FileWriteLine($temp, $lastline)
			_ReplaceStringInFile($temp, $aLines[0] & @CRLF, "")
			FileMove($temp, $file, 1)
		Else
			Local $aLines, $hMatch
			_FileReadToArray($file, $aLines)
			For $i = 1 To $aLines[0]
				If StringInStr($aLines[$i], $userdir_line2) Then $hMatch = 1
			Next
			If $hMatch == 1 Then
				If FileExists($temp) Then FileDelete($temp)
			Else
				If FileExists($temp) Then FileDelete($temp)
				;Local $aLines
				;_FileReadToArray($file, $aLines)
				Local $lastline = _ArrayPop($aLines)
				_FileWriteFromArray($temp, $aLines)
				FileWriteLine($temp, $userdir_line1)
				FileWriteLine($temp, $userdir_line2)
				FileWriteLine($temp, $userdir_line3)
				FileWriteLine($temp, $userdir_line4)
				FileWriteLine($temp, $lastline)
				_ReplaceStringInFile($temp, $aLines[0] & @CRLF, "")
				FileMove($temp, $file, 1)
			EndIf
		EndIf

		_ReplaceStringInFile($file, "  }" & @CRLF & "  {", "  }," & @CRLF & "  {")

	EndIf

EndIf