import sys

for line in sys.stdin:
    s = 0
    if 'areaTemperatureEnd' == line.rstrip():
        break
    if (line.find("maxTemperature=")) == 0:
        s = line[15:23]
        f = float(s)
        print(f)