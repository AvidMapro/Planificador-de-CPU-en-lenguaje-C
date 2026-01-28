import threading
import time

class StepScheduler:
    def __init__(self):
        self._lock = threading.Lock()
        self._events = {}  # tid -> threading.Event
        self._active_tid = None

    def register(self, tid):
        with self._lock:
            self._events[tid] = threading.Event()

    def step(self, tid, action_name):
        # Notify we want to step
        # In a real controlled env, we would wait for explicit permission.
        # For simplicity here: we just log and proceed, or we could wait for a "go" signal from main.
        # This is a "cooperative trace generator" mode.
        with self._lock:
            print(f"[TRACE] T{tid}: requesting {action_name}")

        # Simulate delay to allow interleaving if run normally
        time.sleep(0.01)

    def log(self, tid, msg):
        with self._lock:
            print(f"[LOG] T{tid}: {msg}")

scheduler = StepScheduler()
