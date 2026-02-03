/* error.h -- Error information. */

#ifndef SSQ_ERROR_H
#define SSQ_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum ssq_error_code {
    SSQE_OK = 0,
    SSQE_SYSTEM,
    SSQE_INVALID_RESPONSE,
    SSQE_UNSUPPORTED,
    SSQE_GAI,
    SSQE_NO_SOCKET,
} SSQ_ERROR_CODE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SSQ_ERROR_H */
