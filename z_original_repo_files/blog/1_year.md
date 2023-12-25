# Goldberg Emulator v0.2.5 : 1 Year of Steam Emulator Development

If you just want the download link or more info go to [the Gitlab pages website](https://mr_goldberg.gitlab.io/goldberg_emulator/)

For those that don't know what this is, this is a steam api replacement that lets you play games that use the steam multiplayer apis on LAN without needing steam installed.

It has been a bit more than 1 year since I started working on this steam emulator. I started working on this project on August 3, 2018 and released a first version on August 12, 2018. It has been a much more challenging project than I originally expected. I made it open source on April 13, 2019.

About 1 year ago I decided that I wanted to improve my C++ skills. I had never written any big projects in C++ before, only in C.

In one year with me working on it during my sometimes inexistent free time it has gone from nothing to a very accurate Steam Emulator that works on almost every game.

The main thing I noticed when developing my steam emulator is that too many games seem to be written by people who have no idea what they are doing. They seem to write random code until they get something that works. This leads to some game having convulted behaviors that make me wonder if it's actually part of a DRM scheme that prevent it from working on shitty steam emulators.

Steam userids are stored as a 64bit integer but the part that represents the actual id of the user is a 32bit unsigned integer. However some games (Nioh, Senran Kagura Estival Versus) treat this as a signed int and assume it to be positive. This means if your id is more than 2147483648, bugs in those games start to appear (Nioh crashes and Estival Versus multiplayer doesn't work). There are likely more games that make this mistake. Steam seems to assign ids sequentially so the 2147483648 and next users to create a steam account will likely run into a few weird game breaking bugs.

If you look at the emulator code, you might see a lot of callbacks and behaviours have artificial delays added in. This is because many games depend on those delays and some depend on some operations being always faster than others. For example, the game Ignite calls the logon function on the gameserver interface and the createlobby function on the matchmaking interface at the same time. If for some reason creating a lobby is slower than logging on as a gameserver the created multiplayer game will not be joinable.

I have constantly needed to slow down things in order to fix games.

In the beginning I made the networking functions to have as little delay as possible but that was a mistake because many games (like Age of Empires 2 HD) expect at least some delay.

If valve decides to ever make their steam api faster or change any of the undefined behavior that their api has, many games will break. This means they are stuck and can never change or fix anything. Any change they make has the risk of breaking games that will likely never be updated.

An example of something weird I have had to do is add a microsecond sleep in the SteamAPI_RunCallbacks() function. This function is usually called by the game during every game frame. When I was testing the game, I noticed that the game didn't lag with my slow debug build that prints a lot of stuff to disk while it lagged with my much faster release build. After some tests I hypothesized that it was probably lagging because the thread never yielded to other threads. Of course the std thread yield function didn't do anything but a microsecond sleep completely fixed the lag.

In some games like Senran Kagura Peach Beach Splash when the user leaves a lobby, the game leaves the lobby then right away call some functions on that lobby. If the lobby function returns as if the lobby doesn't exist anymore the game crashes. It turns out that on real steam lobbies stay alive and functions called on them succeed (and act like you are still in the lobby) even if you leave them and there's nobody else in them.

There are also games like School Girl Zombie Hunter that create a lobby and then search for that created lobby. If the search takes less than 3 seconds, the game gives you a nice error that it failed to create the lobby.

Games like No Mans Sky which for some reason creates multiple lobbies when they could have made it much more simple. For anyone that knows about the game it should not come as a surprise that the developers are incompetent. It's clear just looking at how long the game takes to load. The game also has a profanity filter written by top minds which when I tried the game a few months ago for some reason censored the word "noob" but not words like "kike".

An advice if you are a game dev. If you think doing a hash/signature check on the steam api dll will prevent your game from being pirated you are an idiot. The steam emulators made specifically for piracy bypass that easily. Don't do like Risk of Rain 2 that only checks if the steam dll was signed (but not with which signature so a self signed emu is all that's needed to bypass the check).

If you love puzzles which are a real pain in the ass to solve and if you like being frustrated while debugging why some of the most shitty game code ever written by man doesn't work then steam emu development is for you.

As for the future of the emulator thanks to Nemirtingas an overlay might be coming very soon.

### Changelog for 0.2.5

```
v0.2.5
Inventory support added (Thank you Nemirtingas, see the Readme for more info)
Initial Steam Networking Sockets implementation (Fixes Hat in Time Multiplayer)
Updated to sdk 1.46
More accurate auth behavior.
Leaderboards are now configurable (see Readme, some games like Million Arthur or Roof Rage need a steam_settings\leaderboards.txt to work properly).
Various remote storage improvements (Fixed saving/loading in a few games)
A bunch of accuracy improvements and other fixes.
```
