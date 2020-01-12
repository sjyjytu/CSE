#!/usr/bin/python2
import sys
import subprocess

def get_throughput(stdout):
    result_lines = stdout.split("\n")
    words = []
    for line in result_lines:
        if line.find("Aggregated") >= 0:
            words.append(line.split())
    i = 0
    total_throughput = 0.0
    while i < len(words):
        total_throughput += float(words[i][8])
        i += 2
    return total_throughput

def get_results(stdout):
    result_lines = stdout.split("\n")
    words = []
    for line in result_lines:
        if line.find("Aggregated") >= 0:
            words.append(line.split())
    return (words[0][3], words[1][7], words[1][8])




host = sys.argv[1]
testcase = sys.argv[2]

max_load = 100
step = max_load / 10
i = 0

print "Load,Throughput,Avg,95%,99%"
cmd_template = "docker run --rm  dplsming/sockshop-test:0.1 %s %s -t 120 -l %d -v"
while i <= max_load:
    pipes = subprocess.Popen((cmd_template %(host, testcase, i)).split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    std_out, std_err = pipes.communicate()
    throughput = get_throughput(std_out)
    results = get_results(std_err)
    print "%d,%s,%s" %(i,throughput, ",".join(results))
    i += step
