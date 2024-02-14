import json
import datetime

def print_json(data):
    print(json.dumps(data, indent=4))

def get_now_string():
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
