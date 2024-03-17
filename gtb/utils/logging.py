import datetime

def get_now_string() -> str:
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

class Log():
    DEBUG = False
    TRACE = False

    @staticmethod
    def error(msg: str):
        print("[{}] [ERROR] {}".format(get_now_string(), msg))

    @staticmethod
    def exception(msg: str, e: Exception):
        str_err: str = str(e)
        if len(str_err) > 256:
            str_err = str_err[0:256]
        str_err = str_err.replace("\n", "\\n ")
        print("[{}] [ERROR] {}: {}".format(get_now_string(), msg, str_err))

    @staticmethod
    def info(msg: str):
        print("[{}] [INFO] {}".format(get_now_string(), msg))

    @staticmethod
    def debug(msg: str):
        if Log.DEBUG:
            print("[{}] [DEBUG] {}".format(get_now_string(), msg))

    @staticmethod
    def trace(msg: str):
        if Log.TRACE:
            print("[{}] [TRACE] {}".format(get_now_string(), msg))
