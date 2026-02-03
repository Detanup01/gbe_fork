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

#ifndef SETTINGS_PARSER_UFS_INCLUDE_H
#define SETTINGS_PARSER_UFS_INCLUDE_H

#define SI_CONVERT_GENERIC
#define SI_SUPPORT_IOSTREAMS
#define SI_NO_MBCS
#include "simpleini/SimpleIni.h"

#include "settings.h"

void parse_cloud_save(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage);

#endif // SETTINGS_PARSER_UFS_INCLUDE_H
