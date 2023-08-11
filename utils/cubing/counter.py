#!/usr/bin/python3

import sys

x = 0
for line in sys.stdin:
    x += int(line.split(" ")[2])

print(x)
