#!/bin/sh
EXE="./hl2.sh"
EXE_RUN_DIR="$(dirname ${0})"
EXE_COMMAND_LINE="-steam -game cstrike"
APP_ID=240
STEAM_CLIENT_SO=steamclient.so
STEAM_CLIENT64_SO=steamclient64.so
#STEAM_RUNTIME="./steam_runtime/run.sh"

if [ ! -d ~/.steam/sdk32 ]; then
  mkdir -p ~/.steam/sdk32
fi
if [ ! -d ~/.steam/sdk64 ]; then
  mkdir -p ~/.steam/sdk64
fi

if [ ! -f ${STEAM_CLIENT_SO} ]; then
	echo "Couldn't find the requested STEAM_CLIENT_SO."
	exit
fi
if [ ! -f ${STEAM_CLIENT64_SO} ]; then
	echo "Couldn't find the requested STEAM_CLIENT64_SO."
	exit
fi

# for system failure assume orig files are still good
if [ -f ~/.steam/steam.pid.orig ]; then
  mv -f ~/.steam/steam.pid.orig ~/.steam/steam.pid
fi
if [ -f ~/.steam/sdk32/steamclient.so.orig ]; then
  mv -f ~/.steam/sdk32/steamclient.so.orig ~/.steam/sdk32/steamclient.so
fi
if [ -f ~/.steam/sdk64/steamclient.so.orig ]; then
  mv -f ~/.steam/sdk64/steamclient.so.orig ~/.steam/sdk64/steamclient.so
fi

if [ -f ~/.steam/steam.pid ]; then
  mv -f ~/.steam/steam.pid ~/.steam/steam.pid.orig
fi
if [ -f ~/.steam/sdk32/steamclient.so ]; then
  mv -f ~/.steam/sdk32/steamclient.so ~/.steam/sdk32/steamclient.so.orig
fi
if [ -f ~/.steam/sdk64/steamclient.so ]; then
  mv -f ~/.steam/sdk64/steamclient.so ~/.steam/sdk64/steamclient.so.orig
fi

cp ${STEAM_CLIENT_SO} ~/.steam/sdk32/steamclient.so
cp ${STEAM_CLIENT64_SO} ~/.steam/sdk64/steamclient.so
echo ${$} > ~/.steam/steam.pid

cd ${EXE_RUN_DIR}
if [ -z ${STEAM_RUNTIME} ]; then
  SteamAppPath=${EXE_RUN_DIR} SteamAppId=${APP_ID} SteamGameId=${APP_ID} ${EXE} ${EXE_COMMAND_LINE}
else
  SteamAppPath=${EXE_RUN_DIR} SteamAppId=${APP_ID} SteamGameId=${APP_ID} ${STEAM_RUNTIME} ${EXE} ${EXE_COMMAND_LINE}
fi

if [ -f ~/.steam/steam.pid.orig ]; then
  mv -f ~/.steam/steam.pid.orig ~/.steam/steam.pid
fi
if [ -f ~/.steam/sdk32/steamclient.so.orig ]; then
  mv -f ~/.steam/sdk32/steamclient.so.orig ~/.steam/sdk32/steamclient.so
fi
if [ -f ~/.steam/sdk64/steamclient.so.orig ]; then
  mv -f ~/.steam/sdk64/steamclient.so.orig ~/.steam/sdk64/steamclient.so
fi

