from utils.debug import get_now_string

DEBUG=True

class Log():
    @staticmethod
    def error(msg):
        print("[{}] [ERROR] {}".format(get_now_string(), msg))

    @staticmethod
    def info(msg):
        print("[{}] [INFO] {}".format(get_now_string(), msg))

    @staticmethod
    def debug(msg):
        if DEBUG:
            print("[{}] [DEBUG] {}".format(get_now_string(), msg))
