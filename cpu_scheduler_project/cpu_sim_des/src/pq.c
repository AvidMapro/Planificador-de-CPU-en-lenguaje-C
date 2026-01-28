#include <stdlib.h>
#include <string.h>
#include "pq.h"

static int ev_less(const event_t *x, const event_t *y){
    if (x->time != y->time) return x->time < y->time;
    return x->seq < y->seq;
}

void pq_init(event_pq_t *q){
    q->a = NULL; q->n = 0; q->cap = 0;
}

void pq_free(event_pq_t *q){
    free(q->a);
    q->a = NULL; q->n = 0; q->cap = 0;
}

int pq_empty(event_pq_t *q){ return q->n == 0; }

static void sift_up(event_pq_t *q, size_t i){
    while(i > 0){
        size_t p = (i - 1) / 2;
        if (!ev_less(&q->a[i], &q->a[p])) break;
        event_t tmp = q->a[i]; q->a[i] = q->a[p]; q->a[p] = tmp;
        i = p;
    }
}

static void sift_down(event_pq_t *q, size_t i){
    for(;;){
        size_t l = 2*i + 1, r = 2*i + 2, m = i;
        if (l < q->n && ev_less(&q->a[l], &q->a[m])) m = l;
        if (r < q->n && ev_less(&q->a[r], &q->a[m])) m = r;
        if (m == i) break;
        event_t tmp = q->a[i]; q->a[i] = q->a[m]; q->a[m] = tmp;
        i = m;
    }
}

void pq_push(event_pq_t *q, event_t ev){
    if (q->n == q->cap){
        size_t ncap = q->cap ? q->cap * 2 : 64;
        q->a = (event_t*)realloc(q->a, ncap * sizeof(event_t));
        q->cap = ncap;
    }
    q->a[q->n] = ev;
    sift_up(q, q->n);
    q->n++;
}

event_t pq_pop(event_pq_t *q){
    event_t top = q->a[0];
    q->n--;
    if (q->n){
        q->a[0] = q->a[q->n];
        sift_down(q, 0);
    }
    return top;
}
