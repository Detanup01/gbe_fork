#!/usr/bin/env bash

EXE="" #"./hl2.sh"
APP_ID="" #480
EXE_RUN_DIR="" #"$(dirname ${0})"
STEAM_RUNTIME="" #"./steam_runtime/run.sh"
EXE_COMMAND_LINE=()

LDR_FILE_EXE="ldr_exe.txt"
LDR_FILE_APP_ID="ldr_appid.txt"
LDR_FILE_EXE_RUN_DIR="ldr_cwd.txt"
LDR_FILE_STEAM_RUNTIME="ldr_steam_rt.txt"
LDR_FILE_EXE_COMMAND_LINE="ldr_cmd.txt"

# MUST be beside this script
STEAM_CLIENT_SO=steamclient.so
STEAM_CLIENT64_SO=steamclient64.so

script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )
steam_base_dir="$( echo ~/.steam )"

function help_page () {
  echo;
  echo "Command line arguments:"
  echo "  -exe:   path to the executable"
  echo "  -appid: numeric app ID"
  echo "  -cwd:   (optional) working directory to switch to when running the executable"
  echo "  -rt:    (optional) path to Steam runtime script"
  echo "    ex1: $steam_base_dir/debian-installation/ubuntu12_64/steam-runtime-heavy.sh"
  echo "    ex2: $steam_base_dir/debian-installation/ubuntu12_64/steam-runtime-heavy/run.sh"
  echo "    ex3: $steam_base_dir/debian-installation/ubuntu12_32/steam-runtime/run.sh"
  echo "    ex4: $steam_base_dir/steam_runtime/run.sh"
  echo "  --:     everything after these 2 dashes will be passed directly to the executable"
  echo;
  echo "note that any unknown args will be passed to the executable"
  echo;
  echo "Additionally, you can set these values in the equivalent configuration files, everything on a single line for each file:"
  echo "  "$LDR_FILE_EXE": specify the executable path in this file"
  echo "  "$LDR_FILE_APP_ID": specify the app ID in this file"
  echo "  "$LDR_FILE_EXE_RUN_DIR": specify the working directory of the executable path in this file"
  echo "  "$LDR_FILE_STEAM_RUNTIME": specify the path of the Steam runtime script in this file"
  echo "  "$LDR_FILE_EXE_COMMAND_LINE": specify aditional arguments that will be passed to the executable in this file,"
  echo "      this file has the exception that the arguments must be specified on separate lines"
  echo;
  echo "Command line arguments will override the values in the configuration files,"
  echo "with an exception for the arguments passed to the executable, both specified"
  echo "in the cofiguration file and via command line will be passed to the executable"
  echo;
}

# mandatory checks
if [[ ! -f "$script_dir/${STEAM_CLIENT_SO}" ]]; then
  echo "'$STEAM_CLIENT_SO' must be placed beside this script"
  exit 1
fi
if [ ! -f "$script_dir/${STEAM_CLIENT64_SO}" ]; then
  echo "'$STEAM_CLIENT64_SO' must be placed beside this script"
  exit 1
fi

 # no args = help page
if [[ $# = 0 ]]; then
  echo "[?] No arguments provided"
  help_page
  exit 0
fi

# try to load from config files first
# unfortunately we need eval to expand stuff like ~/my_path
if [[ -f "$script_dir/$LDR_FILE_EXE" ]]; then
  read -r -s EXE < "$script_dir/$LDR_FILE_EXE"
  [[ ! -z "$EXE" ]] && EXE="$(eval echo "$EXE")"
fi
if [[ -f "$script_dir/$LDR_FILE_APP_ID" ]]; then
  read -r -s APP_ID < "$script_dir/$LDR_FILE_APP_ID"
fi
if [[ -f "$script_dir/$LDR_FILE_EXE_RUN_DIR" ]]; then
  read -r -s EXE_RUN_DIR < "$script_dir/$LDR_FILE_EXE_RUN_DIR"
  [[ ! -z "$EXE_RUN_DIR" ]] && EXE_RUN_DIR="$(eval echo "$EXE_RUN_DIR")"
fi
if [[ -f "$script_dir/$LDR_FILE_STEAM_RUNTIME" ]]; then
  read -r -s STEAM_RUNTIME < "$script_dir/$LDR_FILE_STEAM_RUNTIME"
  [[ ! -z "$STEAM_RUNTIME" ]] && STEAM_RUNTIME="$(eval echo "$STEAM_RUNTIME")"
fi
if [[ -f "$script_dir/$LDR_FILE_EXE_COMMAND_LINE" ]]; then
  EXTRA_EXE_COMMAND_LINE=()
  readarray -t EXTRA_EXE_COMMAND_LINE < "$script_dir/$LDR_FILE_EXE_COMMAND_LINE"
  for arg in "${EXTRA_EXE_COMMAND_LINE[@]}"; do
    arg="$(eval echo "$arg")"
    EXE_COMMAND_LINE+=("$arg")
  done
fi

# then read command line args
i=1 # 0 is script path
for (( ; i<=$#; i++ )); do
  var="${!i}"
  if [[ "$var" = "-exe" ]]; then
    i=$((i+1))
    EXE="${!i}"
  elif [[ "$var" = "-appid" ]]; then
    i=$((i+1))
    APP_ID="${!i}"
  elif [[ "$var" = "-cwd" ]]; then
    i=$((i+1))
    EXE_RUN_DIR="${!i}"
  elif [[ "$var" = "-rt" ]]; then
    i=$((i+1))
    STEAM_RUNTIME="${!i}"
  elif [[ "$var" = "--" ]]; then # stop processing args, everything else is passed to app
    i=$((i+1))
    break
  else # pass unknown args to app
    EXE_COMMAND_LINE+=("$var")
  fi
done

# extra args after '--' will be processed here
for (( ; i<=$#; i++ )); do
  EXE_COMMAND_LINE+=("${!i}")
done

# sanity checks
# prepending the script dir helps sudo users when for example EXE=./mygame
[[ -f "$EXE" ]] || EXE="$script_dir/$EXE"
if [[ ! -f "$EXE" ]]; then
  echo "[X] executable '$EXE' is invalid" >&2
  help_page
  exit 1
fi
# change to absolute path
EXE="$( cd -- "$( dirname "$EXE" )" 2> /dev/null && pwd )/$( basename "$EXE" )"

if [[ ! "$APP_ID" =~ ^[0-9]+$ ]]; then
  echo "[X] app ID '$APP_ID' is invalid" >&2
  help_page
  exit 1
fi
# use the exe's dir if non-specified
[[ -z "$EXE_RUN_DIR" ]] && EXE_RUN_DIR="$(dirname "$EXE")"
if [[ ! -d "$EXE_RUN_DIR" ]]; then
  echo "[X] executable working directory '$EXE_RUN_DIR' is invalid" >&2
  help_page
  exit 1
fi
if [[ ! -z "$STEAM_RUNTIME" ]]; then
  if [[ ! -f "$STEAM_RUNTIME" ]]; then
    echo "[X] Steam runtime script was set '$STEAM_RUNTIME', but the file was not found" >&2
    help_page
    exit 1
  fi
  # change to absolute path
  STEAM_RUNTIME="$( cd -- "$( dirname "$STEAM_RUNTIME" )" 2> /dev/null && pwd )/$( basename "$STEAM_RUNTIME" )"
fi

# print everything
echo EXE = "'$EXE'"
echo APP_ID = $APP_ID
echo EXE_RUN_DIR = "'$EXE_RUN_DIR'"

if [[ -z "$STEAM_RUNTIME" ]]; then
  echo "STEAM_RUNTIME was not set"
else
  echo STEAM_RUNTIME = "'$STEAM_RUNTIME'"
fi

echo EXE_COMMAND_LINE = "${EXE_COMMAND_LINE[@]}"

# prepare the required dirs
if [[ ! -d "$steam_base_dir/sdk32" ]]; then
  mkdir -p "$steam_base_dir/sdk32"
fi
if [[ ! -d "$steam_base_dir/sdk64" ]]; then
  mkdir -p "$steam_base_dir/sdk64"
fi

# restore earlier backup, we do this here because the user may have crashed the script in an earlier run
# before it could restore the backup at the last stage
if [ -f "$steam_base_dir/steam.pid.orig" ]; then
  mv -f "$steam_base_dir/steam.pid.orig" "$steam_base_dir/steam.pid"
fi
if [ -f "$steam_base_dir/sdk32/steamclient.so.orig" ]; then
  mv -f "$steam_base_dir/sdk32/steamclient.so.orig" "$steam_base_dir/sdk32/steamclient.so"
fi
if [ -f "$steam_base_dir/sdk64/steamclient.so.orig" ]; then
  mv -f "$steam_base_dir/sdk64/steamclient.so.orig" "$steam_base_dir/sdk64/steamclient.so"
fi

# make a backup of original files
if [ -f "$steam_base_dir/steam.pid" ]; then
  mv -f "$steam_base_dir/steam.pid" "$steam_base_dir/steam.pid.orig"
fi
if [ -f "$steam_base_dir/sdk32/steamclient.so" ]; then
  mv -f "$steam_base_dir/sdk32/steamclient.so" "$steam_base_dir/sdk32/steamclient.so.orig"
fi
if [ -f "$steam_base_dir/sdk64/steamclient.so" ]; then
  mv -f "$steam_base_dir/sdk64/steamclient.so" "$steam_base_dir/sdk64/steamclient.so.orig"
fi

# copy our files
cp -f "$script_dir/$STEAM_CLIENT_SO" "$steam_base_dir/sdk32/steamclient.so"
cp -f "$script_dir/$STEAM_CLIENT64_SO" "$steam_base_dir/sdk64/steamclient.so"
echo ${$} > "$steam_base_dir/steam.pid"

# set variables and run the app
pushd "${EXE_RUN_DIR}"

TARGET_EXE="$EXE"
if [ ! -z "${STEAM_RUNTIME}" ]; then
  TARGET_EXE="$STEAM_RUNTIME"
  EXE_COMMAND_LINE=(
    "$EXE"
    "${EXE_COMMAND_LINE[@]}"
  )
fi

SteamAppPath="${EXE_RUN_DIR}" SteamAppId=$APP_ID SteamGameId=$APP_ID SteamOverlayGameId=$APP_ID "$TARGET_EXE" "${EXE_COMMAND_LINE[@]}"

popd

#restore backup
if [ -f "$steam_base_dir/steam.pid.orig" ]; then
  mv -f "$steam_base_dir/steam.pid.orig" "$steam_base_dir/steam.pid"
fi
if [ -f "$steam_base_dir/sdk32/steamclient.so.orig" ]; then
  mv -f "$steam_base_dir/sdk32/steamclient.so.orig" "$steam_base_dir/sdk32/steamclient.so"
fi
if [ -f "$steam_base_dir/sdk64/steamclient.so.orig" ]; then
  mv -f "$steam_base_dir/sdk64/steamclient.so.orig" "$steam_base_dir/sdk64/steamclient.so"
fi

