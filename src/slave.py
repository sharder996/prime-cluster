import json
import math
import multiprocessing
import time
from objects.result import Result
from objects.client import Client
from objects.job import Job


c = Client(9998, "")
num_cores = multiprocessing.cpu_count()

lower = None
mark = None
result = None


def main():
    c.start()
    do_work()
    c.shutdown()


def do_work():
    while True:
        data = objects.Job(**json.loads(c.receive()))

        print("Finding primes from {:,} to {:,}..........".format(data.lower_limit, data.upper_limit), end="")

        start = time.time()
        
        sieve(data.upper_limit, data.lower_limit)
        msg = objects.Result(data.rank, build_msg(), data.upper_limit, data.lower_limit)

        end = time.time()

        print("Done ({:f}sec)".format(end-start))

        c.send(json.dumps(msg.__dict__))


def build_msg():
    msg = []
    i = 0
    for bit in result:
        if bit == 1:
            i += 1
        elif bit == 0:
            msg.append(str(i) + "|")
            i = 1
    return ''.join(msg)[:-1]


def mark_array(k):
    global mark
    index = (-(-lower//k)*k)-lower
    if k == 2:
        for i in range(index, len(mark), k):
            mark[i] = 1
    elif index % 2 == 1:
        for i in range(index, len(mark), 2*k):
            mark[i] = 1
    else:
        if 0 <= index < len(mark):
            mark[index] = 1
        index += k
        for i in range(index, len(mark), 2*k):
            mark[i] = 1


def sieve(upper_bound, lower_bound):
    global mark
    global lower
    global result

    size = upper_bound - lower_bound + 1
    del mark
    mark = multiprocessing.Array('i', size, lock=False)
    klimit = int(math.sqrt(upper_bound)) + 1
    lower = lower_bound

    pool = multiprocessing.Pool(processes=num_cores)
    inputs = list(range(3, klimit+1, 2))
    inputs.insert(0, 2)
    pool.map(mark_array, inputs)
    pool.close()
    pool.join()

    del result
    result = []
    for i in range(size):
        result.append(mark[i])


main()
