#!/bin/python3

# import stl_path
from tokenize import String
from unittest import result
from zipfile import Path
from trex_stl_lib.api import *
import numpy as np

import time
import json
import argparse
import subprocess
from collections import defaultdict

json_output = defaultdict(dict)


def create_pkt_flow(size, ip_min, ip_max, nflows, field):
    print("{} flow will be generated...".format(nflows))
    base_pkt = Ether()/IP(src="16.0.0.1", dst="10.1.3.27")/UDP(dport=1234)
    pad = max(0, size - len(base_pkt)) * 'x'

    vm = STLVM()

    # create a tuple var
    vm.tuple_var(name="tuple", ip_min=ip_min, ip_max=ip_max,
                 port_min=1234, port_max=1234, limit_flows=nflows)

    # write fields
    vm.write(fv_name="tuple.ip", pkt_offset="IP.{}".format(field))
    vm.fix_chksum()

    # vm.write(fv_name="tuple.port", pkt_offset="UDP.sport")

    return STLPktBuilder(pkt=base_pkt/pad, vm=vm)


def create_pkt(size):
    # simple packet creation
    print("Only one flow will be generated...")
    pkt = Ether()/IP(src="10.1.2.27", dst="10.1.3.27")/UDP(dport=1234)
    pad = max(0, size - len(pkt)) * 'x'
    return STLPktBuilder(pkt=pkt/pad)


def simple_burst(streams, m, duration):

    # create client
    # c = STLClient()
    # username/server can be changed those are the default
    # username = common.get_current_user(),
    # server = "localhost"
    c = STLClient(server="localhost", sync_port=1235, async_port=1236)
    passed = True

    try:
        # turn this on for some information
        # c.set_verbose("debug")

        # create one streams
        # s1 = STLStream(packet=create_pkt(size, nflows), mode=STLTXCont())

        # connect to server
        c.connect()

        # prepare our ports (my machine has 0 <--> 1 with static route)
        # Acquire port 0 for $USER
        c.reset(ports=[0, 1])

        # add both streams to ports
        c.add_streams(streams, ports=[0])

        # clear the stats before injecting
        c.clear_stats()

        # set port 1 as promiscuous mode
        c.set_port_attr(ports=[1], promiscuous=True)

        # choose rate and start traffic for 10 seconds on 5 mpps
        print("Running " + m + " on ports 0 for {} seconds...".format(duration))
        c.start(ports=[0], mult=m, duration=duration)

        run_mpstat(duration/2)

        # block until done
        c.wait_on_traffic(ports=[0])

        # read the stats after the test
        stats = c.get_stats()

        # print(json.dumps(stats[0], indent=4, separators=(',', ': '), sort_keys=True))
        # print(json.dumps(stats[1], indent=4, separators=(',', ': '), sort_keys=True))

        # print("\n")
        # print("Packets sent       0 --> 1:   {0} pkts".format(stats[0]["opackets"]))
        # print("Rx Mpps            0 --> 1:   {0} Mpps".format(float(stats[1]["rx_pps"])/1000000))

        json_output[current_test]["throughput"] = float(stats[1]["rx_pps"])/1000000

        if (stats[0]["opackets"] > 100):
            passed = True
        else:
            passed = False

    except STLError as e:
        passed = False
        print(e)

    finally:
        c.disconnect()

    if passed:
        print("\nTest has passed :-)\n")
    else:
        print("\nTest has failed :-(\n")


def run_mpstat(duration):
    global current_test
    cmd = 'ssh india mpstat -P ALL {} 1 -o JSON'.format(int(duration))
    print("Running mpstat... {}".format(cmd))
    output = os.popen(cmd).read()
    # print(json.loads(output))
    json_output[current_test]["mpstat"] = json.loads(output)

def run_ethtool_set_rx_queue(number_rx_queue, password):
    print("Setting NIC rx queue size...")
    ifaces = ["enp3s0f0", "enp3s0f1"]
    for iface in ifaces:
        cmd = 'echo {} | ssh india sudo -S ethtool -L {} combined {}'.format(password, iface, int(number_rx_queue))
        print("Running ethtool... {}".format(cmd))
        os.popen(cmd)
        cmd = 'echo {} | ssh india sudo -S ethtool -l {} '.format(password, iface)
        print("Running ethtool... {}".format(cmd))
        print(os.popen(cmd).read())
        # TODO navarrothiago - Check if it is worth to store this value
        # d[current_test]["rx_queue"] = number_rx_queue
    print("Setting NIC rx queue size...DONE")
    print()

def setup_test_case(name):
    global current_test
    current_test = name
    print("Setup TestCase: {}".format(name))


# Parse the args.
parser = argparse.ArgumentParser()
parser.add_argument('-s',
                    '--size',
                    type=int,
                    default=64,
                    help="The packets length in the stream")
parser.add_argument('-m',
                    '--multiplier',
                    default='100%',
                    help="The throughput in mpps on port 0 (e.g. 14mpps, 90%, 1kbps")
parser.add_argument('-d',
                    '--duration',
                    type=int,
                    default=10,
                    help="The duration of the transmission in second")
parser.add_argument('-f',
                    '--flows',
                    type=int,
                    default='6',
                    help="The number of flows. It must be equal or greater than 6")
parser.add_argument("-a", 
                    "--auto", 
                    help="Ignore all arguments and run in mode automatic",
                    action="store_true")
parser.add_argument('-p',
                    '--password',
                    default="",
                    help="Password of the DUT host")
args = parser.parse_args()

json_output = defaultdict(dict)

current_test = ""
flow_list = [1000]
# flow_list = np.geomspace(1, 1000, num=4, dtype=int)
rx_queue_size_list = np.arange(1, 13, 1)
rx_queue_size_list = np.geomspace(1, 10, num=2, dtype=int)

timestr = time.strftime("%Y%m%d-%H%M%S")
test_case_name="DownlinkMaxThoughtput"

for flow in flow_list:
    # Run the tests
    for rx_size in rx_queue_size_list:
        test_case="{}-{}-{}flow-{}rx".format(timestr, test_case_name, flow, rx_size)
        with open("tests/reports/{}.json".format(test_case), "w") as dump_file:
            run_ethtool_set_rx_queue(rx_size, args.password)
            setup_test_case("{}".format(test_case))
            s1 = STLStream(packet=create_pkt_flow(args.size, "16.0.0.1",
                        "16.0.0.254", int(flow), "src"), mode=STLTXCont())
            simple_burst([s1], args.multiplier, args.duration)
            json.dump(json_output[test_case]["mpstat"], dump_file, indent=2, separators=(',', ': '), sort_keys=True)
