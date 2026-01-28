import threading
import time
import random
from scheduler import scheduler

# Configuration
BUFFER_SIZE = 5
N_PRODUCERS = 2
N_CONSUMERS = 2
ITEMS_PER_PRODUCER = 5

buffer = []
mutex = threading.Lock()
empty = threading.Semaphore(BUFFER_SIZE)
full = threading.Semaphore(0)

def producer(tid):
    scheduler.register(tid)
    for i in range(ITEMS_PER_PRODUCER):
        item = f"item_{tid}_{i}"

        scheduler.step(tid, "waiting_empty")
        empty.acquire()

        scheduler.step(tid, "waiting_mutex")
        with mutex:
            scheduler.step(tid, "producing")
            buffer.append(item)
            scheduler.log(tid, f"Produced {item} | Buffer: {len(buffer)}")

        full.release()
        scheduler.step(tid, "released_full")
        time.sleep(random.random() * 0.1)

def consumer(tid):
    scheduler.register(tid)
    for i in range(ITEMS_PER_PRODUCER):
        scheduler.step(tid, "waiting_full")
        full.acquire()

        scheduler.step(tid, "waiting_mutex")
        with mutex:
            scheduler.step(tid, "consuming")
            if buffer:
                item = buffer.pop(0)
                scheduler.log(tid, f"Consumed {item} | Buffer: {len(buffer)}")

        empty.release()
        scheduler.step(tid, "released_empty")
        time.sleep(random.random() * 0.1)

threads = []
for i in range(N_PRODUCERS):
    t = threading.Thread(target=producer, args=(i,))
    threads.append(t)

for i in range(N_CONSUMERS):
    t = threading.Thread(target=consumer, args=(i+N_PRODUCERS,))
    threads.append(t)

for t in threads: t.start()
for t in threads: t.join()

print("Simulation finished.")
