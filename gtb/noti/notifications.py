from gtb.core.thread import BotThread
from gtb.core.context import Context
from gtb.noti.sms import send_message

class NotificationThread(BotThread):

    def __init__(self, ctx: Context) -> None:
        super().__init__(ctx)

    def init(self) -> None:
        self.ctx.notify.queue("Ghw Trade Bot starting up")

    def think(self) -> None:
        msg: str | None = self.ctx.notify.peek()
        if msg and self.notify(msg):
            self.ctx.notify.pop()

    def notify(self, msg: str) -> bool:
        return send_message(msg)
