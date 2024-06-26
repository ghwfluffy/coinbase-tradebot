import math

def floor(x, decimals) -> float:
    power = float(pow(10, decimals))
    return math.floor(float(x) * power) / power

def floor_btc(x) -> float:
    return floor(x, 8)

def floor_percent(x) -> float:
    return floor(x, 4)

def floor_usd(x) -> float:
    return floor(x, 2)

def floor_seconds(x) -> float:
    return floor(x, 2)
