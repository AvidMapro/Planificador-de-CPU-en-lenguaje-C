import threading
import time
import random
from scheduler import scheduler

# Reader-Writer with Writer Priority (starvation possible for readers)
# Variables
read_count = 0
mutex_rc = threading.Lock()
wrt = threading.Lock()

def reader(tid):
    global read_count
    scheduler.register(tid)
    for _ in range(3):
        scheduler.step(tid, "enter_read_try")
        with mutex_rc:
            read_count += 1
            if read_count == 1:
                wrt.acquire()
                scheduler.log(tid, "First reader locked wrt")

        # Reading section
        scheduler.step(tid, "reading")
        scheduler.log(tid, f"Reading... (readers={read_count})")
        time.sleep(0.05)

        with mutex_rc:
            read_count -= 1
            if read_count == 0:
                wrt.release()
                scheduler.log(tid, "Last reader unlocked wrt")

        time.sleep(random.random() * 0.2)

def writer(tid):
    scheduler.register(tid)
    for _ in range(3):
        scheduler.step(tid, "enter_write_try")
        wrt.acquire()

        # Writing section
        scheduler.step(tid, "writing")
        scheduler.log(tid, "WRITING!!!")
        time.sleep(0.1)

        wrt.release()
        scheduler.step(tid, "exit_write")
        time.sleep(random.random() * 0.2)

threads = []
for i in range(3): threads.append(threading.Thread(target=reader, args=(i,)))
for i in range(2): threads.append(threading.Thread(target=writer, args=(i+3,)))

for t in threads: t.start()
for t in threads: t.join()
