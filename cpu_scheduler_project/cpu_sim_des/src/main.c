#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"

static void usage(const char *p){
    fprintf(stderr,
      "Uso: %s --alg fcfs|srtf|prio_rr --in processes.csv --gantt gantt.txt --events events.csv [--q 4]\n", p);
}

int main(int argc, char **argv){
    const char *alg_s = NULL, *in = NULL, *gantt = "gantt.txt", *events = "events.csv";
    int q = 4;

    for (int i=1;i<argc;i++){
        if (!strcmp(argv[i],"--alg") && i+1<argc) alg_s = argv[++i];
        else if (!strcmp(argv[i],"--in") && i+1<argc) in = argv[++i];
        else if (!strcmp(argv[i],"--gantt") && i+1<argc) gantt = argv[++i];
        else if (!strcmp(argv[i],"--events") && i+1<argc) events = argv[++i];
        else if (!strcmp(argv[i],"--q") && i+1<argc) q = atoi(argv[++i]);
        else { usage(argv[0]); return 1; }
    }
    if (!alg_s || !in){ usage(argv[0]); return 1; }

    alg_t alg;
    if (!strcmp(alg_s,"fcfs")) alg = ALG_FCFS;
    else if (!strcmp(alg_s,"srtf")) alg = ALG_SRTF;
    else if (!strcmp(alg_s,"prio_rr")) alg = ALG_PRIO_RR;
    else { usage(argv[0]); return 1; }

    pcb_t *P = NULL; size_t n = 0;
    if (!sim_load_csv(in, &P, &n) || n == 0){
        fprintf(stderr, "Error leyendo CSV.\n");
        return 1;
    }

    sim_t S;
    memset(&S, 0, sizeof(S));
    S.alg = alg;
    S.quantum = q;
    S.procs = P;
    S.nprocs = n;
    S.next_seq = 1;

    int ok = sim_run(&S, gantt, events);

    free(P);
    return ok ? 0 : 1;
}
