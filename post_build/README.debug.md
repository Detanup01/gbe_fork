## What is this ?
This is the debug build of the emu, while the game/app is running,  the emu will write various events to a log file called `STEAM_LOG.txt`

## Where is this log file ?
It should be beside the `.dll/.so` itself, unless the environment variable `GseAppPath` is defined, in which case this will be the path of this log file

## Why ?
This is intended for debugging purposes, use it to check the behavior of the emu while running
