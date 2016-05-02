#!/usr/bin/env python3

# Automatic grapher for final proj
# CJ Guttormsson

import matplotlib.pyplot as plt
import subprocess
from tqdm import tqdm

def one_run(which_result, *args):
    """Run the blockchain-sim program with same args as this function"""
    string =  subprocess.check_output(["./blockchain-sim"] +
                                      list(args)).decode('utf-8')
    lines = string.split('\n')

    # a better way to do this would be a pattern search, but I'm hardcoding
    # locations for now
    num_blocks = int(lines[4].split()[3])
    num_transactions = int(lines[5].split()[3])
    avg_ttc = float(lines[6].split()[2])
    avg_fee = float(lines[7].split()[3])
    percent_confirmed = float(lines[4].split()[3])

    if which_result == 0:
        return avg_ttc
    if which_result == 1:
        return avg_fee
    if which_result == 2:
        return percent_confirmed

def avg_of_runs(nruns, which_result, *args):
    """Take n runs with args and average which_result"""
    return sum(one_run(which_result, *args) for _ in tqdm(range(nruns))) / nruns

def graph_it(x_list, y_list, labels, indep, dep):
    """Take input list, output list, and axis labels, and make a graph"""
    plt.plot(x_list, y_list)
    plt.xlabel(labels[indep]['x'])
    plt.ylabel(labels[dep]['y'])
    plt.title(labels[dep]['y'] + " vs. " + labels[indep]['x'])
    plt.grid(True)
    plt.savefig(labels[dep]['y'] + "_vs_" + labels[indep]['x'] + ".png")
    #plt.show()
    
    
# Example set
def main(argv):
    # Read in command line parameters
    indep_var = int(argv[1])
    dep_var   = int(argv[2])
    nruns     = int(argv[3])

    # set other params
    # min_connectivitiy, mean_tx_interarrival, mean_block_interarrival, mean_link_speed
    
    base = (4, 10, 100, 2)
    connectivity_varied  = range(1, 11, 1)
    mean_tx_itarv_varied = range(5, 101, 5)
    mean_link_speed      = range(10, 101, 10)

    if indep_var == 0:
        varied = connectivity_varied
        independents = [{
            'min_connectivity' : str(x),
            "mean_tx_interarrival" : "10",  
            "mean_block_interarrival" : "100",
            "mean_link_speed": "2" }
                        for x in connectivity_varied]
    elif indep_var == 1:
        varied = mean_tx_itarv_varied
        independents = [{
            'min_connectivity' : "4",
            "mean_tx_interarrival" : str(x),  
            "mean_block_interarrival" : "100",
            "mean_link_speed": "2" }
                        for x in mean_tx_itarv_varied]
    elif indep_var == 2:
        varied = mean_link_speed
        independents = [{
            'min_connectivity' : "4",
            "mean_tx_interarrival" : "10",  
            "mean_block_interarrival" : "100",
            "mean_link_speed": str(x) }
                        for x in mean_link_speed]


    results = []
    for varset in tqdm(independents):
        results.append(avg_of_runs(nruns, dep_var, varset['min_connectivity'], varset['mean_tx_interarrival'],
                                   varset['mean_block_interarrival'], varset['mean_link_speed']))

    # TODO: make this dynamic
    labels = [{'x':'Minimum Connectivity', 'y':'Average Time to Confirmation'},
              {'x':'Mean Transaction Interarrival Time', 'y':'Average Fee'},
              {'x':'Mean Link Latency', 'y':'Percent of Transactions Confirmed'}]
    print(results)
    graph_it(varied, results, labels, indep_var, dep_var)

if __name__ == '__main__':
    import sys
    main(sys.argv)
