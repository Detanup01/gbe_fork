/* server.h -- Representation of a Source server. */

#ifndef SSQ_SERVER_H
#define SSQ_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#ifdef _WIN32
# include <ws2tcpip.h>
#else /* !_WIN32 */
# include <sys/time.h>
#endif /* _WIN32 */

#include "ssq/error.h"

#ifndef SSQ_TIMEOUT_RECV_DEFAULT
# define SSQ_TIMEOUT_RECV_DEFAULT 5000 // ms
#endif /* !SSQ_TIMEOUT_RECV_DEFAULT */
#ifndef SSQ_TIMEOUT_SEND_DEFAULT
# define SSQ_TIMEOUT_SEND_DEFAULT 5000 // ms
#endif /* !SSQ_TIMEOUT_SEND_DEFAULT */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct ssq_server SSQ_SERVER;

typedef enum ssq_timeout_selector {
    SSQ_TIMEOUT_RECV = (1 << 0),
    SSQ_TIMEOUT_SEND = (1 << 1),
} SSQ_TIMEOUT_SELECTOR;

SSQ_SERVER    *ssq_server_new(const char *hostname, uint16_t port);
void           ssq_server_free(SSQ_SERVER *server);

#ifdef _WIN32
void           ssq_server_timeout(SSQ_SERVER *server, SSQ_TIMEOUT_SELECTOR which, DWORD value_in_ms);
#else /* !_WIN32 */
void           ssq_server_timeout(SSQ_SERVER *server, SSQ_TIMEOUT_SELECTOR which, time_t value_in_ms);
#endif /* _WIN32 */

bool           ssq_server_eok(const SSQ_SERVER *server);
SSQ_ERROR_CODE ssq_server_ecode(const SSQ_SERVER *server);
const char    *ssq_server_emsg(const SSQ_SERVER *server);
void           ssq_server_eclr(SSQ_SERVER *server);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SSQ_SERVER_H */
