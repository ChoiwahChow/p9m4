#!/usr/bin/python3

import sys

x = 0
for line in sys.stdin:
    x += float(line.split(",")[0].split("=")[1])

print(x)
