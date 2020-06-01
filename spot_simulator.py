#!/usr/bin/env python

#
# Author: xxx
# Full training with typical spot-vm crash/restore pattern
#

import os
import psutil
import subprocess
import time
import csv
import math
from statistics import mean
from random import randint

# NB: test with more crashes; the more the better
PROCNAME = "app-plinius"
BINPATH = "./"
LOSS = BINPATH + "results/loss.csv"
TRACES = BINPATH + "traces/price_time_c3.2xlarge.csv"
RESULTS = BINPATH + "results/spot_trace.csv"
MAX_ITERS = 500
MAX_LOSS = 2.36

# spot price update frequency = 5 mins
UPDATE_FREQ = 300000

#TEST = 60000

# current iteration
cur_iter = 0
loss = 0.0

# time counter: each 1 increment represents 5 mins
counter = 0
# read the prices
prices = []
temp = []
with open(TRACES, 'r') as file:
    reader = csv.reader(file, delimiter=",")
    # temp = list(zip(*reader))[0]
    for row in reader:
        # print("\nPrice: ",float(row[0]))
        prices.append(float(row[0]))


# set a max_bid price
# max_bid = mean(prices)
max_bid = 0.095500  # We chose a value a little higher than the median value = 0.094
print("My max bid price: ", max_bid, "\n")

# results list
results = []


# read current iteration
def get_cur_iter():
    with open(LOSS, "r") as file:
        # read last line
        for row in reversed(list(csv.reader(file, delimiter=","))):
            return int(row[0])


# get loss
def get_loss():
    with open(LOSS, "r") as file:
        # read last line
        for row in reversed(list(csv.reader(file, delimiter=","))):
            return float(row[1])


# write results
def register_vals():
    with open(RESULTS, "a", newline='') as res_file:
        writer = csv.writer(res_file, delimiter=',')
        writer.writerow(results)
    # res_file.close()
    results.clear()


# stop/resume training
length = len(prices)
running = False
start = 0
num_stops = 0  # number of times the training process was stopped
state = 0  # state of the training process: 1 = on ; 0 = off
stop_index = []
while ((counter <= length) and (cur_iter < 500)):

    if(prices[counter] > max_bid):
        print("Bid price lower than market price, killing training process\n")
        for proc in psutil.process_iter():
            # if the process exists kill it
            if proc.name() == PROCNAME:
                proc.kill()
                # wait for the program to exit and the mmap to clean
                time.sleep(0.100)
                #running = False
                num_stops += 1
                stop_index.append(cur_iter)
                state = 0
    else:
        print("Bid price greater than market price, running training process\n")
        start += 1
        if (state == 0):
            subprocess.Popen([BINPATH + PROCNAME])
            # start += 1
            state = 1
            #running == True

    # sleep for spot update period
    time.sleep(UPDATE_FREQ/1000.)
    # time.sleep(TEST/1000.)
    if(start > 1):  # prevents reading from an empty file
        loss = get_loss()
    else:
         # training has not begun yet so loss is actually undefined but we leave it at MAX_LOSS
        loss = MAX_LOSS

    results = [counter*5, state, loss]
    register_vals()
    counter += 1

    if(start > 1):
        cur_iter = get_cur_iter()
        # cur_iter *= 1
    else:
        cur_iter = 0

    print("\nCurrent iteration: ", cur_iter, "Counter: ", counter)

# Add last point after training completes
results = [counter*5, 0, loss]
register_vals()
print("Crash iters: ", stop_index)
