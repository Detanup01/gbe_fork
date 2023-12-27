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

#include "base.h"

class Steam_MusicRemote : public ISteamMusicRemote
{
public:
	// Service Definition
 	bool RegisterSteamMusicRemote( const char *pchName );
 	bool DeregisterSteamMusicRemote();
	bool BIsCurrentMusicRemote();
	bool BActivationSuccess( bool bValue );

	bool SetDisplayName( const char *pchDisplayName );
	bool SetPNGIcon_64x64( void *pvBuffer, uint32 cbBufferLength );
	
	// Abilities for the user interface
	bool EnablePlayPrevious(bool bValue);
	bool EnablePlayNext( bool bValue );
	bool EnableShuffled( bool bValue );
	bool EnableLooped( bool bValue );
	bool EnableQueue( bool bValue );
	bool EnablePlaylists( bool bValue );

	// Status
 	bool UpdatePlaybackStatus( AudioPlayback_Status nStatus );
	bool UpdateShuffled( bool bValue );
	bool UpdateLooped( bool bValue );
	bool UpdateVolume( float flValue ); // volume is between 0.0 and 1.0

	// Current Entry
	bool CurrentEntryWillChange();
	bool CurrentEntryIsAvailable( bool bAvailable );
	bool UpdateCurrentEntryText( const char *pchText );
	bool UpdateCurrentEntryElapsedSeconds( int nValue );
	bool UpdateCurrentEntryCoverArt( void *pvBuffer, uint32 cbBufferLength );
	bool CurrentEntryDidChange();

	// Queue
	bool QueueWillChange();
	bool ResetQueueEntries();
	bool SetQueueEntry( int nID, int nPosition, const char *pchEntryText );
	bool SetCurrentQueueEntry( int nID );
	bool QueueDidChange();

	// Playlist
	bool PlaylistWillChange();
	bool ResetPlaylistEntries();
	bool SetPlaylistEntry( int nID, int nPosition, const char *pchEntryText );
	bool SetCurrentPlaylistEntry( int nID );
	bool PlaylistDidChange();
};
