# CPU Scheduler and Synchronization Lab

Proyecto académico de Sistemas Operativos que combina un simulador de planificación de CPU en C con ejercicios de sincronización en Python. El simulador utiliza eventos discretos, recibe procesos desde CSV y genera una traza Gantt y un registro de eventos reproducible.

## Algoritmos implementados

- FCFS First Come First Served
- SRTF Shortest Remaining Time First
- Priority Round Robin con quantum configurable

En Priority Round Robin, un valor numérico mayor representa una prioridad mayor.

## Compilación

Requisitos: compilador compatible con C11 y `make`.

```bash
cd cpu_scheduler_project/cpu_sim_des
make
```

El comando genera el ejecutable `cpu_sim`.

## Ejecución

```bash
./cpu_sim --alg fcfs --in processes.csv --gantt gantt.txt --events events.csv
./cpu_sim --alg srtf --in processes.csv --gantt gantt.txt --events events.csv
./cpu_sim --alg prio_rr --in processes.csv --gantt gantt.txt --events events.csv --q 4
```

Parámetros principales:

| Parámetro | Descripción |
| --- | --- |
| `--alg` | `fcfs`, `srtf` o `prio_rr` |
| `--in` | Archivo CSV de procesos |
| `--gantt` | Archivo de salida con la traza de ejecución |
| `--events` | Archivo CSV con los eventos de la simulación |
| `--q` | Quantum de Priority Round Robin; valor predeterminado `4` |

## Formato de entrada

El CSV utiliza este encabezado:

```csv
pid,arrival,burst,priority
```

- `pid`: identificador del proceso.
- `arrival`: instante de llegada.
- `burst`: tiempo de CPU requerido.
- `priority`: prioridad del proceso.

El repositorio incluye `processes.csv` como entrada de ejemplo.

## Laboratorio de sincronización

El módulo Python contiene simulaciones con trazas controladas de dos problemas clásicos:

- productor consumidor;
- lectores escritores.

```bash
cd cpu_scheduler_project/sync_lab_py
python3 producer_consumer.py
python3 readers_writers.py
```

## Estructura

```text
cpu_scheduler_project/
  cpu_sim_des/
    Makefile
    processes.csv
    src/                 Simulador y cola de prioridad
  sync_lab_py/
    producer_consumer.py
    readers_writers.py
    scheduler.py
  presentation/
    slides.html
```

## Conceptos demostrados

- simulación de eventos discretos;
- planificación apropiativa y no apropiativa;
- colas de prioridad y quantum;
- generación de trazas para analizar decisiones del planificador;
- exclusión mutua, coordinación de hilos y acceso compartido.
