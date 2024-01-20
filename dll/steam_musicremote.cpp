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

#include "dll/steam_musicremote.h" 

// Service Definition
bool Steam_MusicRemote::RegisterSteamMusicRemote( const char *pchName )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::DeregisterSteamMusicRemote()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::BIsCurrentMusicRemote()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::BActivationSuccess( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}


bool Steam_MusicRemote::SetDisplayName( const char *pchDisplayName )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::SetPNGIcon_64x64( void *pvBuffer, uint32 cbBufferLength )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}


// Abilities for the user interface
bool Steam_MusicRemote::EnablePlayPrevious(bool bValue)
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::EnablePlayNext( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::EnableShuffled( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::EnableLooped( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::EnableQueue( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::EnablePlaylists( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}


// Status
bool Steam_MusicRemote::UpdatePlaybackStatus( AudioPlayback_Status nStatus )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::UpdateShuffled( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::UpdateLooped( bool bValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::UpdateVolume( float flValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}
 // volume is between 0.0 and 1.0

// Current Entry
bool Steam_MusicRemote::CurrentEntryWillChange()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::CurrentEntryIsAvailable( bool bAvailable )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::UpdateCurrentEntryText( const char *pchText )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::UpdateCurrentEntryElapsedSeconds( int nValue )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::UpdateCurrentEntryCoverArt( void *pvBuffer, uint32 cbBufferLength )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::CurrentEntryDidChange()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}


// Queue
bool Steam_MusicRemote::QueueWillChange()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::ResetQueueEntries()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::SetQueueEntry( int nID, int nPosition, const char *pchEntryText )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::SetCurrentQueueEntry( int nID )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::QueueDidChange()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}


// Playlist
bool Steam_MusicRemote::PlaylistWillChange()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::ResetPlaylistEntries()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::SetPlaylistEntry( int nID, int nPosition, const char *pchEntryText )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::SetCurrentPlaylistEntry( int nID )
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}

bool Steam_MusicRemote::PlaylistDidChange()
{
    PRINT_DEBUG("TODO Steam_MusicRemote::%s\n", __FUNCTION__);
    return false;
}
