#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sim.h"
#include "pq.h"

#define MAXP  64        // máximo nivel de prioridad soportado (0..MAXP)
#define MAXLINE 512

// ---------- util: CSV ----------
static int parse_int(const char *s, int *out){
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s) return 0;
    *out = (int)v;
    return 1;
}

int sim_load_csv(const char *path, pcb_t **out, size_t *n_out){
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    pcb_t *a = NULL;
    size_t n = 0, cap = 0;

    char line[MAXLINE];
    int first = 1;
    while (fgets(line, sizeof(line), f)){
        // strip
        size_t L = strlen(line);
        while (L && (line[L-1]=='\n' || line[L-1]=='\r')) line[--L]=0;
        if (!L) continue;

        // skip header if present
        if (first){
            first = 0;
            if (strstr(line, "pid") && strstr(line, "arrival")) continue;
        }

        // tokenize
        char *tok[4] = {0};
        int k = 0;
        char *p = line;
        while (k < 4){
            tok[k++] = p;
            char *c = strchr(p, ',');
            if (!c) break;
            *c = 0;
            p = c + 1;
        }
        if (k < 4) continue;

        int pid, arr, burst, prio;
        if (!parse_int(tok[0], &pid)) continue;
        if (!parse_int(tok[1], &arr)) continue;
        if (!parse_int(tok[2], &burst)) continue;
        if (!parse_int(tok[3], &prio)) continue;

        if (n == cap){
            size_t ncap = cap ? cap*2 : 64;
            a = (pcb_t*)realloc(a, ncap * sizeof(pcb_t));
            cap = ncap;
        }
        a[n].pid = pid;
        a[n].arrival = arr;
        a[n].burst = burst;
        a[n].priority = prio;

        a[n].remaining = burst;
        a[n].first_start = -1;
        a[n].finish = -1;
        a[n].last_start = -1;
        n++;
    }
    fclose(f);

    *out = a;
    *n_out = n;
    return 1;
}

// ---------- ready structures ----------
// Implementación simple (sin dependencias): listas por pid.
typedef struct node { int pid; struct node *next; } node_t;

static void push_back(node_t **h, node_t **t, int pid){
    node_t *x = (node_t*)malloc(sizeof(node_t));
    x->pid = pid; x->next = NULL;
    if (*t) (*t)->next = x; else *h = x;
    *t = x;
}

static int pop_front(node_t **h, node_t **t){
    node_t *x = *h;
    int pid = x->pid;
    *h = x->next;
    if (!*h) *t = NULL;
    free(x);
    return pid;
}

static int empty(node_t *h){ return h == NULL; }

static void free_list(node_t *h){
    while(h){
        node_t *n = h->next;
        free(h);
        h = n;
    }
}

// SRTF ready: lista ordenada por remaining asc, tie: arrival asc, pid asc
static void srtf_insert(node_t **h, int pid, pcb_t *P){
    node_t *x = (node_t*)malloc(sizeof(node_t));
    x->pid = pid; x->next = NULL;

    node_t **cur = h;
    while(*cur){
        int q = (*cur)->pid;
        if (P[pid].remaining < P[q].remaining) break;
        if (P[pid].remaining == P[q].remaining){
            if (P[pid].arrival < P[q].arrival) break;
            if (P[pid].arrival == P[q].arrival && P[pid].pid < P[q].pid) break;
        }
        cur = &(*cur)->next;
    }
    x->next = *cur;
    *cur = x;
}

// ---------- helpers ----------
static pcb_t* by_pid(sim_t *S, int pid){
    return &S->procs[pid]; // pid es indice 0..N-1
}

static event_t ev_make(sim_t *S, int time, ev_type_t type, int pid){
    event_t e;
    e.time = time;
    e.seq = S->next_seq++;
    e.type = type;
    e.pid = pid;
    return e;
}

static int all_done(sim_t *S){
    for (size_t i=0;i<S->nprocs;i++){
        if (S->procs[i].finish < 0) return 0;
    }
    return 1;
}

// ---------- scheduler decisions ----------
typedef struct {
    // FCFS/RR
    node_t *q_h, *q_t;

    // SRTF
    node_t *srtf_h;

    // PRIO_RR: colas por prioridad
    node_t *prio_h[MAXP+1];
    node_t *prio_t[MAXP+1];
} ready_t;

static void ready_init(ready_t *R){
    memset(R, 0, sizeof(*R));
}
static void ready_free(ready_t *R){
    free_list(R->q_h);
    free_list(R->srtf_h);
    for(int p=0;p<=MAXP;p++) free_list(R->prio_h[p]);
}

static void ready_add(sim_t *S, ready_t *R, int pid){
    pcb_t *p = by_pid(S, pid);
    if (S->alg == ALG_FCFS){
        push_back(&R->q_h, &R->q_t, pid);
    } else if (S->alg == ALG_SRTF){
        srtf_insert(&R->srtf_h, pid, S->procs);
    } else if (S->alg == ALG_PRIO_RR){
        int pr = p->priority;
        if (pr < 0) pr = 0;
        if (pr > MAXP) pr = MAXP;
        push_back(&R->prio_h[pr], &R->prio_t[pr], pid);
    }
}

static int ready_pick_next(sim_t *S, ready_t *R){
    if (S->alg == ALG_FCFS){
        if (empty(R->q_h)) return -1;
        return pop_front(&R->q_h, &R->q_t);
    } else if (S->alg == ALG_SRTF){
        if (empty(R->srtf_h)) return -1;
        node_t *x = R->srtf_h;
        int pid = x->pid;
        R->srtf_h = x->next;
        free(x);
        return pid;
    } else { // PRIO_RR
        for (int pr = MAXP; pr >= 0; pr--){
            if (!empty(R->prio_h[pr])){
                return pop_front(&R->prio_h[pr], &R->prio_t[pr]);
            }
        }
        return -1;
    }
}

static int current_prio(sim_t *S){
    if (S->running_pid < 0) return -999999;
    return by_pid(S, S->running_pid)->priority;
}

// ---------- main sim ----------
int sim_run(sim_t *S, const char *gantt_path, const char *events_csv_path){
    event_pq_t Q; pq_init(&Q);
    ready_t R; ready_init(&R);

    FILE *fg = fopen(gantt_path, "w");
    FILE *fe = fopen(events_csv_path, "w");
    if (!fg || !fe){ if(fg)fclose(fg); if(fe)fclose(fe); return 0; }

    fprintf(fe, "time,event,pid,info\n");

    for (size_t i=0;i<S->nprocs;i++){
        pq_push(&Q, ev_make(S, S->procs[i].arrival, EV_ARRIVAL, (int)i));
    }
    pq_push(&Q, ev_make(S, 0, EV_DISPATCH, -1));

    S->running_pid = -1;
    S->now = 0;

    while(!pq_empty(&Q) && !all_done(S)){
        event_t ev = pq_pop(&Q);
        S->now = ev.time;

        if (ev.type == EV_ARRIVAL){
            ready_add(S, &R, ev.pid);
            fprintf(fe, "%d,ARRIVAL,%d,\n", S->now, ev.pid);

            if (S->running_pid >= 0){
                if (S->alg == ALG_SRTF){
                    pcb_t *run = by_pid(S, S->running_pid);
                    int ran = S->now - run->last_start;
                    if (ran > 0) run->remaining -= ran;

                    pcb_t *np = by_pid(S, ev.pid);
                    if (np->remaining < run->remaining){
                        ready_add(S, &R, S->running_pid);
                        fprintf(fe, "%d,PREEMPT,%d,by=%d\n", S->now, S->running_pid, ev.pid);
                        S->running_pid = -1;
                        pq_push(&Q, ev_make(S, S->now, EV_DISPATCH, -1));
                    } else {
                        run->last_start = S->now;
                    }
                } else if (S->alg == ALG_PRIO_RR){
                    int pr_new = by_pid(S, ev.pid)->priority;
                    int pr_run = current_prio(S);
                    if (pr_new > pr_run){
                        pcb_t *run = by_pid(S, S->running_pid);
                        int ran = S->now - run->last_start;
                        if (ran > 0) run->remaining -= ran;

                        ready_add(S, &R, S->running_pid);
                        fprintf(fe, "%d,PREEMPT,%d,by=%d\n", S->now, S->running_pid, ev.pid);
                        S->running_pid = -1;
                        pq_push(&Q, ev_make(S, S->now, EV_DISPATCH, -1));
                    }
                }
            }
        }
        else if (ev.type == EV_DISPATCH){
            if (S->running_pid >= 0) continue; 
            int pid = ready_pick_next(S, &R);
            if (pid < 0) continue;

            pcb_t *p = by_pid(S, pid);
            if (p->first_start < 0) p->first_start = S->now;
            p->last_start = S->now;
            S->running_pid = pid;

            fprintf(fg, "%d..", S->now);
            fprintf(fe, "%d,DISPATCH,%d,\n", S->now, pid);

            int slice = p->remaining;
            if (S->alg == ALG_PRIO_RR) slice = (slice > S->quantum) ? S->quantum : slice;

            if (S->alg == ALG_PRIO_RR && slice < p->remaining){
                pq_push(&Q, ev_make(S, S->now + slice, EV_QUANTUM_EXPIRE, pid));
            } else {
                pq_push(&Q, ev_make(S, S->now + slice, EV_COMPLETE, pid));
            }
        }
        else if (ev.type == EV_QUANTUM_EXPIRE){
            if (S->running_pid != ev.pid) continue; 
            pcb_t *p = by_pid(S, ev.pid);
            int ran = S->now - p->last_start;
            if (ran > 0) p->remaining -= ran;

            fprintf(fg, "%d:P%d | ", S->now, p->pid);
            fprintf(fe, "%d,Q_EXPIRE,%d,\n", S->now, ev.pid);

            S->running_pid = -1;
            ready_add(S, &R, ev.pid);
            pq_push(&Q, ev_make(S, S->now, EV_DISPATCH, -1));
        }
        else if (ev.type == EV_COMPLETE){
            if (S->running_pid != ev.pid) continue;
            pcb_t *p = by_pid(S, ev.pid);
            int ran = S->now - p->last_start;
            if (ran > 0) p->remaining -= ran;
            if (p->remaining < 0) p->remaining = 0;

            p->finish = S->now;
            fprintf(fg, "%d:P%d | ", S->now, p->pid);
            fprintf(fe, "%d,COMPLETE,%d,\n", S->now, ev.pid);

            S->running_pid = -1;
            pq_push(&Q, ev_make(S, S->now, EV_DISPATCH, -1));
        }
    }

    fprintf(fg, "END\n");

    fprintf(fe, "time,event,pid,info\n"); 
    for (size_t i=0;i<S->nprocs;i++){
        pcb_t *p = &S->procs[i];
        int tat = p->finish - p->arrival;
        int wt  = tat - p->burst;
        int rt  = p->first_start - p->arrival;
        fprintf(fe, "%d,STATS,%d,turnaround=%d;waiting=%d;response=%d;finish=%d\n",
                S->now, p->pid, tat, wt, rt, p->finish);
    }

    fclose(fg);
    fclose(fe);
    ready_free(&R);
    pq_free(&Q);
    return 1;
}
