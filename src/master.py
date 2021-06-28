import json
import objects
import os
import sys
import threading
from datetime import datetime
from objects.server import Server
from queue import Queue


INTERVAL = 500000000   # Size of interval to check  #100000000#
INDENT = 19  # Number to change how many numbers wide the primes are written
JOB_TIME_LIMIT = 1200 # Number of seconds that the server will expect a client to finish a task in

FILENAME_PREFIX = "primes_"
FILE_TYPE = ".txt"
FILE_PATH = "primes/"
FILE_SIZE_LIMIT = 524288000

NUMBER_OF_THREADS = 4
JOB_NUMBER = [0,1,2,3]


threads = []
thread_pool = Queue()
primes_file = None
s = Server(9998, "")
exit_flag = False


# Load config from file
config_file = open("config.txt", "r")
config = config_file.readlines()
config_file.close()
starting_num = int(config[0][config[0].find(":")+1:])   # Number to start finding primes from
num_of_primes = int(config[1][config[1].find(":")+1:])  # Number of primes found so far
current_indent = int(config[2][config[2].find(":")+1:]) # Starting indentation in file
file_suffix = int(config[3][config[3].find(":")+1:])    # Filename suffix for storing primes
largest_prime = int(config[4][config[4].find(":")+1:])  # Largest prime found so far
filename = FILE_PATH + FILENAME_PREFIX + str(file_suffix) + FILE_TYPE


task_queue = []
in_progress_tasks = []
received_data = []
queued_interval = 0
current_interval = 0
scheduled_num = starting_num
current_num = starting_num


def main():
    s.start()
    create_workers()
    for t in threads:
        t.join()
    s.shutdown()


def create_workers():
    for x in JOB_NUMBER:
        thread_pool.put(x)
    for _ in range(NUMBER_OF_THREADS):
        t = threading.Thread(target=work)
        t.daemon = True
        t.start()
        threads.append(t)


def work():
    while not exit_flag:
        x = thread_pool.get()
        if x == 0:
            send()
        if x == 1:
            receive()
        if x == 2:
            data_management()
        if x == 3:
            ui()


def send():
    while not exit_flag:
        if queued_interval - current_interval < s.connections:
            create_tasks(s.connections)

        for (task, time) in in_progress_tasks:
            if (datetime.now() - time).total_seconds() > JOB_TIME_LIMIT:
                if remove_task(in_progress_tasks, task):
                    ordered_insert(task_queue, task)
        
        s.send(task_queue, in_progress_tasks)


def receive():
    while not exit_flag:
        received_jobs = []
        s.receive(received_jobs)
        for data in received_jobs:
            result = objects.Result(**(json.loads(data)))
            if remove_task(in_progress_tasks, result):
                ordered_insert(received_data, result)


def remove_task(list, task):
    i = 0
    while i < len(list) and list[i][0].rank != task.rank:
        i += 1
    if i < len(list) and list[i][0].rank == task.rank:
        del list[i]
        return True
    else:
        return False


def ordered_insert(list, item):
    i = 0
    while i < len(list) and list[i].rank < item.rank:
        i += 1
    list.insert(i, item)


def data_management():
    global current_interval
    global primes_file
    primes_file = open(filename, "a+")

    while not exit_flag:
        if len(received_data) > 0:
            data = received_data[0]
            if data.rank == current_interval+1:
                del received_data[0]
                current_interval += 1
                print_results(data)
    primes_file.close()
    updateConfig()


def create_tasks(count):
    global scheduled_num
    global queued_interval

    for i in range(count):
        queued_interval += 1
        lower = scheduled_num
        scheduled_num += INTERVAL
        task = objects.Job(scheduled_num-1, lower, queued_interval)
        ordered_insert(task_queue, task)


def print_results(data):
    global current_indent
    global num_of_primes
    global current_num
    global file_suffix
    global filename
    global primes_file
    global largest_prime

    i = 0
    offsets = data.payload.split('|')
    for offset in offsets:
        i += int(offset)
        if current_indent > INDENT:
            current_indent = 0
            primes_file.write("\n")
            if os.path.getsize(filename) > FILE_SIZE_LIMIT:
                primes_file.flush()
                primes_file.close()
                file_suffix += 1
                filename = FILE_PATH + FILENAME_PREFIX + str(file_suffix) + FILE_TYPE
                primes_file = open(filename, "w+")
        primes_file.write(str(i + data.lower_limit) + " ")
        num_of_primes += 1
        largest_prime = i + data.lower_limit
        current_indent += 1
    primes_file.flush()
    current_num = data.upper_limit + 1


def ui():
    while not exit_flag:
        command = input()
        if command == "status":
            print_status()
        elif command == "stop":
            stop()
        elif command == "queue":
            backlog()
        else:
            print("Invalid command!")


def backlog():
    print("       # of queued jobs: %d" % len(task_queue))
    print("      # of pending jobs: %d" % len(in_progress_tasks))
    print("# of jobs to be written: %d" % len(received_data))


def print_status():
    print("# of connected clients: %d" % s.connections)
    print("     # of primes found: %s" % ("{:,}".format(num_of_primes)))
    print("      # searched up to: %s" % ("{:,}".format(current_num)))
    print("    Order of magnitude: %d" % (len(str(current_num))-1))
    print("   Largest prime found: %s" % ("{:,}".format(largest_prime)))


def updateConfig():
    config_file = open("config.txt", "w")
    config_file.write("starting_num:" + str(current_num) + "\n")
    config_file.write("num_of_primes:" + str(num_of_primes) + "\n")
    config_file.write("current_indent:" + str(current_indent) + "\n")
    config_file.write("file_suffix:" + str(file_suffix) + "\n")
    config_file.write("largest_prime:" + str(largest_prime) + "\n")
    config_file.close()


def stop():
    global exit_flag
    exit_flag = True


main()


print('Successfully exited server!')
