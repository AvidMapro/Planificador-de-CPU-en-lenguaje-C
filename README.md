# CPU Scheduler & Sync Lab

## 1. CPU Scheduler (C)
Simulador DES (Discrete Event Simulation) de algoritmos de CPU.

### Compilar
```bash
cd cpu_sim_des
make
```

### Ejecutar
```bash
./cpu_sim --alg fcfs --in processes.csv --gantt gantt.txt --events events.csv
./cpu_sim --alg srtf --in processes.csv --gantt gantt.txt --events events.csv
./cpu_sim --alg prio_rr --in processes.csv --gantt gantt.txt --events events.csv --q 4
```

### Formato CSV
`pid,arrival,burst,priority` (Mayor número = mayor prioridad)

## 2. Sync Lab (Python)
Simulación de problemas de sincronización con trazas controladas.

```bash
cd sync_lab_py
python producer_consumer.py
python readers_writers.py
```
