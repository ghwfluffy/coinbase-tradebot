def floor(x, decimals):
    power = float(pow(10, decimals))
    return math.floor(x * power) / power
