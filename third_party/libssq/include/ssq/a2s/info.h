/** info.h -- A2S_INFO server queries. */

#ifndef SSQ_A2S_INFO_H
#define SSQ_A2S_INFO_H

#include <stddef.h>

#include "ssq/server.h"

#define A2S_INFO_FLAG_GAMEID   0x01
#define A2S_INFO_FLAG_KEYWORDS 0x20
#define A2S_INFO_FLAG_PORT     0x80
#define A2S_INFO_FLAG_STEAMID  0x10
#define A2S_INFO_FLAG_STV      0x40

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum a2s_server_type {
    A2S_SERVER_TYPE_DEDICATED     = 'd',
    A2S_SERVER_TYPE_NON_DEDICATED = 'l',
    A2S_SERVER_TYPE_STV_RELAY     = 'p',
    A2S_SERVER_TYPE_UNKNOWN       = '\0',
} A2S_SERVER_TYPE;

typedef enum a2s_environment {
    A2S_ENVIRONMENT_LINUX   = 'l',
    A2S_ENVIRONMENT_WINDOWS = 'w',
    A2S_ENVIRONMENT_MAC     = 'm',
    A2S_ENVIRONMENT_UNKNOWN = '\0',
} A2S_ENVIRONMENT;

typedef struct a2s_info {
    uint8_t         protocol;     /* Protocol version used by the server.                      */
    char           *name;         /* Name of the server.                                       */
    size_t          name_len;     /* Length of the `name' string.                              */
    char           *map;          /* Map the server has currently loaded.                      */
    size_t          map_len;      /* Length of the `map' string.                               */
    char           *folder;       /* Name of the folder containing the game files.             */
    size_t          folder_len;   /* Length of the `folder' string.                            */
    char           *game;         /* Full name of the game.                                    */
    size_t          game_len;     /* Length of the `game' string.                              */
    uint16_t        id;           /* Steam Application ID of game.                             */
    uint8_t         players;      /* Number of players on the server.                          */
    uint8_t         max_players;  /* Maximum number of players the server reports it can hold. */
    uint8_t         bots;         /* Number of bots on the server.                             */
    A2S_SERVER_TYPE server_type;  /* The type of server.                                       */
    A2S_ENVIRONMENT environment;  /* The operating system of the server.                       */
    bool            visibility;   /* Whether the server requires a password.                   */
    bool            vac;          /* Whether the server uses VAC.                              */
    char           *version;      /* Version of the game installed on the server.              */
    size_t          version_len;  /* Length of the `version' string.                           */
    uint8_t         edf;          /* Extra Data Flag.                                          */
    uint16_t        port;         /* The server's game port number.                            */
    uint64_t        steamid;      /* Server's SteamID.                                         */
    uint16_t        stv_port;     /* Spectator port number for SourceTV.                       */
    char           *stv_name;     /* Name of the spectator server for SourceTV.                */
    size_t          stv_name_len; /* Length of the `stv_name' string.                          */
    char           *keywords;     /* Tags that describe the game according to the server.      */
    size_t          keywords_len; /* Length of the `keywords' string.                          */
    uint64_t        gameid;       /* The server's 64-bit GameID.                               */
} A2S_INFO;

A2S_INFO *ssq_info(SSQ_SERVER *server);
void      ssq_info_free(A2S_INFO *info);

bool      ssq_info_has_gameid(const A2S_INFO *info);
bool      ssq_info_has_keywords(const A2S_INFO *info);
bool      ssq_info_has_port(const A2S_INFO *info);
bool      ssq_info_has_steamid(const A2S_INFO *info);
bool      ssq_info_has_stv(const A2S_INFO *info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SSQ_A2S_INFO_H */
