#!/usr/bin/python3

# import time;time.sleep(20);
file = open("Makefile")
for line in file.readlines():
    print(line, flush=True)
