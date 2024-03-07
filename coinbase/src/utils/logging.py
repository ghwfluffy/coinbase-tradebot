import datetime

DEBUG=True

def get_now_string():
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

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
