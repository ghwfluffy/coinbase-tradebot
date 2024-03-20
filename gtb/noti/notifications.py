import random

from datetime import datetime
from dateutil.relativedelta import relativedelta

from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.noti.sms import send_message

class NotificationThread(BotThread):
    last_failure: datetime
    cooldown: int

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx, sleep_seconds=5)
        self.last_failure = datetime.now() - relativedelta(hours=1)
        self.cooldown = 0

    def init(self) -> None:
        self.ctx.notify.queue("Ghw Trade Bot starting up")

    def think(self) -> None:
        msg: str | None = self.ctx.notify.peek()
        if msg and self.notify(msg):
            self.ctx.notify.pop()

    def notify(self, msg: str) -> bool:
        # Don't spam if we got an error
        if self.last_failure > datetime.now() - relativedelta(minutes=self.cooldown):
            return False
        if send_message(msg):
            return True
        self.last_failure = datetime.now()
        self.cooldown = random.randint(10, 15)
        return False
