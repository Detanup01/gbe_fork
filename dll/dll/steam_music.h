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

class Steam_Music : public ISteamMusic
{
    int playing;
    float volume;
    void change_playstate(int new_playing);

    class SteamCallBacks *callbacks;
public:
    Steam_Music(class SteamCallBacks *callbacks);

	bool BIsEnabled();
	bool BIsPlaying();
	
	AudioPlayback_Status GetPlaybackStatus(); 

	void Play();
	void Pause();
	void PlayPrevious();
	void PlayNext();

	// volume is between 0.0 and 1.0
	void SetVolume( float flVolume );
	float GetVolume();
};
