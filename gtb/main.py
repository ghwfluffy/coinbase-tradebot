import signal
from functools import partial

from gtb.core.bot import Bot
from gtb.core.version import VERSION
from gtb.utils.logging import Log

def run_bot() -> None:
    Log.info("Starting version {}".format(VERSION))

    bot: Bot = Bot()

    # Shut down cleanly on SIGINT/SIGKILL
    signal.signal(signal.SIGINT, lambda s,f: bot.stop())
    signal.signal(signal.SIGTERM, lambda s,f: bot.stop())

    # Run bot threads
    bot.start()
    while bot.is_running():
        bot.wait()

    # Wait for bot threads to exit
    Log.info("Shutting down")
    bot.join()
    Log.info("Exiting")

if __name__ == "__main__":
    Log.DEBUG = True
    Log.TRACE = True
    run_bot()
