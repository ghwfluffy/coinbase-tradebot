from threading import Condition
from threading import Thread

from gtb.core.context import Context
from gtb.core.thread import BotThread

from gtb.market.current import CurrentMarketThread
from gtb.orders.processor import OrderProcessor
from gtb.phases.tracker import PhaseTracker
from gtb.traders.hodl import DiamondHands
from gtb.traders.spread import SpreadTrader

class Bot():
    ctx: Context
    cond: Condition
    threads: list[BotThread]

    def __init__(self) -> None:
        self.ctx = Context()
        self.threads = [
            # Keep current market conditions updated
            CurrentMarketThread(self.ctx),
            # Keep current market conditions updated
            OrderProcessor(self.ctx),
            # Continuously make determinations if we're going up or down
            PhaseTracker(self.ctx),
            # Periodically buy and "hodl"
            DiamondHands(self.ctx),
            # Pick buy and sell points based on current market
            SpreadTrader(self.ctx),
            # Try to predict large up ticks in the market
            #PhasedTrader(self.ctx),
        ]
        self.cond = Condition()

    def start(self) -> None:
        # Initialize from filesystem state
        self.ctx.history.read_fs()
        self.ctx.order_book.read_fs()

        self.ctx.is_running = True

        # Start threads
        for t in self.threads:
            t.init()
            t.start()

    def stop(self) -> None:
        self.ctx.is_running = False
        with self.cond:
            self.cond.notify()

    def join(self) -> None:
        self.stop()
        for t in self.threads:
            t.join()

    def wait(self) -> None:
        with self.cond:
            self.cond.wait()

    def is_running(self) -> bool:
        return self.ctx.is_running
