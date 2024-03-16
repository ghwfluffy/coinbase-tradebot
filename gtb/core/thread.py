import time

from threading import Thread
from abc import ABC, abstractmethod

from gtb.core.context import Context

class BotThread(Thread):
    ctx: Context

    def __init__(self, ctx: Context) -> None:
        super().__init__()
        self.ctx = ctx

    @abstractmethod
    def init(self) -> None:
        pass

    @abstractmethod
    def think(self) -> None:
        pass

    def run(self) -> None:
        while self.ctx.is_running:
            self.think()
            time.sleep(1)
