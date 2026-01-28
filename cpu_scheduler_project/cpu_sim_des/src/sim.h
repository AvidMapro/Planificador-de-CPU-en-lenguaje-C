#ifndef SIM_H
#define SIM_H

#include <stddef.h>

typedef enum { ALG_FCFS=1, ALG_SRTF=2, ALG_PRIO_RR=3 } alg_t;

typedef struct {
    int pid;
    int arrival;
    int burst;
    int priority;

    int remaining;

    int first_start;   // -1 si nunca corrió
    int finish;        // -1 si no terminó

    int last_start;    // para contabilizar tramo ejecutado
} pcb_t;

typedef enum {
    EV_ARRIVAL=1,
    EV_DISPATCH=2,
    EV_QUANTUM_EXPIRE=3,
    EV_COMPLETE=4
} ev_type_t;

typedef struct {
    int time;
    unsigned long seq;     // desempate determinista
    ev_type_t type;
    int pid;               // pid asociado (si aplica)
} event_t;

typedef struct {
    // config
    alg_t alg;
    int quantum;

    // time
    int now;
    unsigned long next_seq;

    // CPU
    int running_pid;       // -1 si idle

    // arrays
    pcb_t *procs;
    size_t nprocs;
} sim_t;

// API
int sim_load_csv(const char *path, pcb_t **out, size_t *n_out);
int sim_run(sim_t *S, const char *gantt_path, const char *events_csv_path);

#endif
