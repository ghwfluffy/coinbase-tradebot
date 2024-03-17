import time

from threading import Thread
from abc import ABC, abstractmethod

from gtb.core.context import Context

class BotThread(Thread):
    ctx: Context
    sleep_seconds: float

    def __init__(self, ctx: Context, sleep_seconds: float = 1.0) -> None:
        super().__init__()
        self.ctx = ctx
        self.sleep_seconds = sleep_seconds

    @abstractmethod
    def init(self) -> None:
        pass

    @abstractmethod
    def think(self) -> None:
        pass

    def run(self) -> None:
        while self.ctx.is_running:
            self.think()
            if self.sleep_seconds > 0:
                time.sleep(self.sleep_seconds)
