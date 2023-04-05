from trex_stl_lib.api import *
from scapy.contrib.gtp import *

class STLS1(object):

    def create_stream (self):
       return STLStream(
           packet =
                   STLPktBuilder(
                       pkt = Ether()/IP(src="192.168.60.1",dst="192.168.60.4")/
                               UDP(dport=2152)/GTP_U_Header(teid=100)/IP(src="10.10.10.10",dst="192.168.128.5",version=4)/UDP()/Raw('x'*20)
                   ),
           mode = STLTXCont())

    def get_streams (self, direction = 0, **kwargs):
       # create 1 stream
       return [ self.create_stream() ]


# dynamic load - used for trex console or simulator
def register():
   return STLS1()

# def create_stream (self):
#        return STLStream(
#            packet =
#                    STLPktBuilder(
#                        pkt = Ether()/IP(src="192.168.60.5",dst="192.168.60.4")/
#                                UDP(dport=2152)/GTP_U_Header(teid=100)/IP(src="10.10.10.10",dst="192.168.128.5",version=4)/UDP()/Raw('x'*20)
#                    ),
#            mode = STLTXCont())

#     def get_streams (self, direction = 0, **kwargs):
#        # create 1 stream
#        return [ self.create_stream() ]


# # dynamic load - used for trex console or simulator
# def register():
#    return STLS1()
# 