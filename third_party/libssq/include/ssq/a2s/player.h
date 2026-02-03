/* player.h -- A2S_PLAYER server queries. */

#ifndef SSQ_A2S_PLAYER_H
#define SSQ_A2S_PLAYER_H

#include <stddef.h>

#include "ssq/server.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct a2s_player {
    uint8_t index;    /* Index of player chunk starting from 0.                     */
    char   *name;     /* Name of the player.                                        */
    size_t  name_len; /* Length of the `name' string.                               */
    int32_t score;    /* Player's score (usually "frags" or "kills").               */
    float   duration; /* Time (in seconds) player has been connected to the server. */
} A2S_PLAYER;

A2S_PLAYER *ssq_player(SSQ_SERVER *server, uint8_t *player_count);
void        ssq_player_free(A2S_PLAYER *players, uint8_t player_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SSQ_A2S_PLAYER_H */
