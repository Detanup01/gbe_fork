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

#include "dll/steam_music.h" 

Steam_Music::Steam_Music(class SteamCallBacks *callbacks)
{
    playing = 0;
    volume = 1.0;
    this->callbacks = callbacks;
}

void Steam_Music::change_playstate(int new_playing)
{
    if (new_playing != playing) {
        PlaybackStatusHasChanged_t data;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }

    playing = new_playing;
}

bool Steam_Music::BIsEnabled()
{
    PRINT_DEBUG("Steam_Music::BIsEnabled\n");
    return true;
}

bool Steam_Music::BIsPlaying()
{
    PRINT_DEBUG("Steam_Music::BIsPlaying\n");
    return playing > 0;
}


AudioPlayback_Status Steam_Music::GetPlaybackStatus()
{
    PRINT_DEBUG("Steam_Music::GetPlaybackStatus\n");
    if (playing == 0) {
        return AudioPlayback_Idle;
    }

    if (playing == 1) {
        return AudioPlayback_Paused;
    }

    if (playing == 2) {
        return AudioPlayback_Playing;
    }

    return AudioPlayback_Idle;
}

void Steam_Music::Play()
{
    PRINT_DEBUG("Steam_Music::Play\n");
    change_playstate(2);
}

void Steam_Music::Pause()
{
    PRINT_DEBUG("Steam_Music::Pause\n");
    change_playstate(1);
}

void Steam_Music::PlayPrevious()
{
    PRINT_DEBUG("Steam_Music::PlayPrevious\n");
    change_playstate(2);
}

void Steam_Music::PlayNext()
{
    PRINT_DEBUG("Steam_Music::PlayNext\n");
    change_playstate(2);
}


// volume is between 0.0 and 1.0
void Steam_Music::SetVolume( float flVolume )
{
    PRINT_DEBUG("Steam_Music::SetVolume\n");

    if (flVolume > 1.0)
        flVolume = 1.0;
    if (flVolume < 0.0)
        flVolume = 0.0;

    if (flVolume != volume) {
        VolumeHasChanged_t data;
        data.m_flNewVolume = volume;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }

    volume = flVolume;
}

float Steam_Music::GetVolume()
{
    PRINT_DEBUG("Steam_Music::GetVolume\n");
    return volume;
}
