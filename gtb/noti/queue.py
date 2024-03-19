from typing import List

from threading import Lock

class NotificationQueue():
    message_queue: List[str]
    mtx: Lock
    def __init__(self) -> None:
        self.mtx = Lock()
        self.message_queue = []

    def queue(self, msg: str) -> None:
        with self.mtx:
            self.message_queue.append(msg)

    def peek(self) -> str | None:
        with self.mtx:
            if len(self.message_queue) > 0:
                return self.message_queue[0]
            return None

    def pop(self) -> str | None:
        with self.mtx:
            if len(self.message_queue) > 0:
                ret = self.message_queue[0]
                self.message_queue.pop(0)
                return ret
            return None
