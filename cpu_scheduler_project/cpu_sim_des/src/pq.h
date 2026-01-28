#ifndef PQ_H
#define PQ_H

#include "sim.h"

typedef struct {
    event_t *a;
    size_t n;
    size_t cap;
} event_pq_t;

void pq_init(event_pq_t *q);
void pq_free(event_pq_t *q);
int  pq_empty(event_pq_t *q);
void pq_push(event_pq_t *q, event_t ev);
event_t pq_pop(event_pq_t *q);

#endif
