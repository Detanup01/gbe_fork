/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "dll/steam_applist.h"

uint32 Steam_Applist::GetNumInstalledApps()
{
    PRINT_DEBUG("TODO Steam_Applist::GetNumInstalledApps\n");
    return 0;
}

uint32 Steam_Applist::GetInstalledApps( AppId_t *pvecAppID, uint32 unMaxAppIDs )
{
    PRINT_DEBUG("TODO Steam_Applist::GetInstalledApps\n");
    return 0;
}

// returns -1 if no name was found
int  Steam_Applist::GetAppName( AppId_t nAppID, STEAM_OUT_STRING() char *pchName, int cchNameMax )
{
    PRINT_DEBUG("TODO Steam_Applist::GetAppName\n");
    return -1;
}

// returns -1 if no dir was found
int  Steam_Applist::GetAppInstallDir( AppId_t nAppID, char *pchDirectory, int cchNameMax )
{
    PRINT_DEBUG("TODO Steam_Applist::GetAppInstallDir\n");
    return -1;
}

// return the buildid of this app, may change at any time based on backend updates to the game
int Steam_Applist::GetAppBuildId( AppId_t nAppID )
{
    PRINT_DEBUG("TODO Steam_Applist::GetAppBuildId\n");
    return 10;
}
