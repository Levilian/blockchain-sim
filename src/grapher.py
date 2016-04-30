#!/usr/bin/env python3

# Automatic grapher for final proj
# CJ Guttormsson

import matplotlib.pyplot as plt
import subprocess
from tqdm import tqdm

def one_run(*args):
    string =  subprocess.check_output(["./blockchain-sim"] + list(args)).decode('utf-8')
    lines = string.split('\n')

    num_blocks = int(lines[4].split()[3])
    num_transactions = int(lines[5].split()[3])
    avg_ttc = float(lines[6].split()[2])
    avg_fee = float(lines[7].split()[3])
    percent_confirmed = float(lines[4].split()[3])
    
    return avg_ttc

def avg_of_runs(nruns, *args):
    allruns = []
    for _ in tqdm(range(nruns)):
        allruns.append(one_run(*args))
    
    return sum(allruns) / nruns

def graph_it(x_list, y_list, labels):
    plt.plot(x_list, y_list)
    plt.xlabel(labels['x'])
    plt.ylabel(labels['y'])
    plt.title(labels['y'] + " vs. " + labels['x'])
    plt.grid(True)
    plt.savefig("plot.png")
    plt.show()
    
    
# Example set
if __name__ == '__main__':
    AVG_OF = 5

    varied = list(range(5, 101, 5))
    
    independents = [{
        'min_connectivity' : "4",
        "mean_tx_interarrival" : str(x),  
        "mean_block_interarrival" : "100",
        "mean_link_speed": "2" }
                    for x in varied]


    results = []
    for varset in tqdm(independents):
        results.append(avg_of_runs(AVG_OF, varset['min_connectivity'], varset['mean_tx_interarrival'],
                                   varset['mean_block_interarrival'], varset['mean_link_speed']))

    # TODO: make this dynamic
    labels = {'x':'Min Transaction Interarrival Time', 'y':'Average Time to Confirmation'}
    print(results)
    graph_it(varied, results, labels)
