/* rules.h -- A2S_RULES server queries. */

#ifndef SSQ_A2S_RULES_H
#define SSQ_A2S_RULES_H

#include <stddef.h>

#include "ssq/server.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct a2s_rules {
    char   *name;      /* Name of the rule.             */
    size_t  name_len;  /* Length of the `name' string.  */
    char   *value;     /* Value of the rule.            */
    size_t  value_len; /* Length of the `value' string. */
} A2S_RULES;

A2S_RULES *ssq_rules(SSQ_SERVER *server, uint16_t *rule_count);
void       ssq_rules_free(A2S_RULES *rules, uint16_t rule_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SSQ_A2S_RULES_H */
