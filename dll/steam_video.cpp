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

#include "dll/steam_video.h" 

// Get a URL suitable for streaming the given Video app ID's video
void Steam_Video::GetVideoURL( AppId_t unVideoAppID )
{
    PRINT_DEBUG("TODO Steam_Video::GetVideoURL\n");
}


// returns true if user is uploading a live broadcast
bool Steam_Video::IsBroadcasting( int *pnNumViewers )
{
    PRINT_DEBUG("TODO Steam_Video::IsBroadcasting\n");
    return false;
}


// Get the OPF Details for 360 Video Playback
STEAM_CALL_BACK( GetOPFSettingsResult_t )
void Steam_Video::GetOPFSettings( AppId_t unVideoAppID )
{
    PRINT_DEBUG("TODO Steam_Video::GetOPFSettings\n");
}

bool Steam_Video::GetOPFStringForApp( AppId_t unVideoAppID, char *pchBuffer, int32 *pnBufferSize )
{
    PRINT_DEBUG("TODO Steam_Video::GetOPFStringForApp\n");
    return false;
}
