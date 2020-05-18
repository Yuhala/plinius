#!/usr/bin/env python
import psutil
import subprocess
import time
from random import randint

# NB: test with more crashes; the more the better
PROCNAME = "app"
BINPATH = "/home/ubuntu/xxx/sgx-dnet-romulus/"
#BINPATH = "/tensorflow/bazel-bin/tensorflow/cc/heart-model/"


NUMTIMES = 200


for i in range(1,NUMTIMES):
    subprocess.Popen([BINPATH + PROCNAME])
    irand = randint(4000, 5000)    # if the time is too short, we get multiple processes fire up at the same time
    print("iteration ",i," +++++ Crashing the process ",NUMTIMES," times, with ",irand," miliseconds delay +++++")
    time.sleep(irand/1000.)
    end = True
    for proc in psutil.process_iter():
        # check whether the process name matches
        if proc.name() == PROCNAME:
            proc.kill()
            time.sleep(0.100)     # wait for the program to exit and the mmap to clean

