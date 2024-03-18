import datetime

from threading import Lock

def get_now_string() -> str:
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

class Log():
    INFO: bool = True
    DEBUG: bool = False
    TRACE: bool = False
    mtx: Lock = Lock()

    @staticmethod
    def error(msg: str) -> None:
        with Log.mtx:
            print("[{}] [ERROR] {}".format(get_now_string(), msg))

    @staticmethod
    def exception(msg: str, e: Exception) -> None:
        str_err: str = str(e)
        if len(str_err) > 256:
            str_err = str_err[0:256]
        str_err = str_err.replace("\n", "\\n ")
        with Log.mtx:
            print("[{}] [ERROR] {}: {}".format(get_now_string(), msg, str_err))

    @staticmethod
    def info(msg: str) -> None:
        if Log.INFO:
            with Log.mtx:
                print("[{}] [INFO] {}".format(get_now_string(), msg))

    @staticmethod
    def debug(msg: str) -> None:
        if Log.DEBUG:
            with Log.mtx:
                print("[{}] [DEBUG] {}".format(get_now_string(), msg))

    @staticmethod
    def trace(msg: str) -> None:
        if Log.TRACE:
            with Log.mtx:
                print("[{}] [TRACE] {}".format(get_now_string(), msg))
