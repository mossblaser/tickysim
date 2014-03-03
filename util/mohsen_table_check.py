#!/usr/bin/env python

"""
Interpret/varify router configs for Mohsen's router tables.

XXX: A total hack!
"""

def chunk(lst, chunk_size):
	for i in xrange(0, len(lst), chunk_size):
		yield lst[i:i+chunk_size]


import sys

import topology

filename = sys.argv[1]

num_chips = int(sys.argv[2])

data_file = [int(n,16) for n in open(filename,"r").read().strip().split()]

assert data_file[-1] == 0xBBBB \
     , "File not 0xBBBB terminated!"

assert (len(data_file)-1) % (num_chips+5) == 0 \
     , "File should have one terminating block and then sections of length num_chips+5"

assert (((len(data_file)-1) % (num_chips+5))**0.5)%1.0 == 0 \
     , "File should have a squre number of entries!"

system_size = int(((len(data_file)-1) / (num_chips+5))**0.5)
def index_to_pos(index):
	x = index%system_size
	y = index/system_size
	return (x,y)
def pos_to_id(pos):
	return pos[0]<<4 | pos[1]
def id_to_pos(id):
	return (id>>4, id&0xF)


# Is this a single board system?
single_board = system_size == 8

# Values used to check global settings are applied consistently
global_sample_period = None
global_timeout = None
global_injection_rate = None
global_consumption_rate = None


def is_in_board(pos):
	x = pos[0]-4
	y = pos[1]-3
	return (x,y) in list(topology.hexagon())


def check_first_dest(pos, first_dest):
	"""
	XXX: For cyclic traffic in a torus only.
	
	Check that the first destination of the given chip is one to the right.
	"""
	# Don't check if the node isn't on the board
	if not is_in_board(pos):
		return
	
	x = pos[0] + 1
	y = pos[1]
	
	if x >= system_size:
		x = 0
		y += 1
	
	if y >= system_size:
		y = 0
	
	if not single_board or is_in_board((x,y)):
		assert (x,y) == first_dest \
		     , "Core does not start sending packets to the node to its right."
	else:
		# If not in the board, just move on until we're back in the board
		check_first_dest((x,y), first_dest)


def check_num_dests(pos, num_dests):
	"""
	XXX: For cyclic traffic in a torus only.
	"""
	if single_board and is_in_board(pos):
		assert 48-1 == num_dests \
		     , "Wrong number of destinations."
	elif not single_board:
		assert (system_size*system_size) - 1 == num_dests \
		     , "Wrong number of destinations."
	else:
		# Num dests can be anything because this chip is not on the baord
		pass


# Go over each router's entry
for chip_index, data_chunk in enumerate(chunk(data_file[:-1], num_chips+5)):
	chip_pos = index_to_pos(chip_index)
	
	# Produce routing table files in the same form as SpiNNaker for external
	# varification.
	routing_table = data_chunk[:num_chips]
	with open("%s.%d.%d"%(filename, chip_pos[0], chip_pos[1]), "w") as f:
		for route in routing_table:
			# Check extra bit is zero
			assert route&0x80 == 0 \
			     , "Unused bit 7 in routing entry is not zero!"
			
			x,y = id_to_pos(route>>8)
			# XXX: X and Y are the wrong way around as in Javier's files
			f.write("0x%08X, 0x%08X\n"%((y<<8|x), route & 0x7F))
	
	# Check that the first destination is appropriate for the traffic pattern
	first_dest = data_chunk[num_chips + 0] >> 8
	check_first_dest(chip_pos, id_to_pos(first_dest))
	
	# Check the Chip ID is correct
	cur_chip_id = data_chunk[num_chips + 0] & 0x00FF
	assert cur_chip_id == pos_to_id(chip_pos) \
	     , "Chip given wrong ID"
	
	# Check the zero field before the number of destinations
	assert data_chunk[num_chips + 1] >> 8 == 0 \
	     , "Zero field is non-zero!"
	
	# Check that the number of destinations is consistent with the traffic pattern
	num_dests = data_chunk[num_chips + 1] & 0xFF
	check_num_dests(chip_pos, num_dests)
	
	# Check consumption rate is consistent throughout
	consumption_rate = data_chunk[num_chips + 2] >> 10
	if single_board and not is_in_board(chip_pos):
		assert consumption_rate == 0 \
		     , "Consumption rate is non-zero for node outside board"
	else:
		if global_consumption_rate is None:
			global_consumption_rate = consumption_rate
		assert global_consumption_rate == consumption_rate \
		     , "Consumption rate is not the same accross the whole system"
	
	# Check consumption rate is consistent throughout
	injection_rate = data_chunk[num_chips + 2] & 0x03FF
	if single_board and not is_in_board(chip_pos):
		assert injection_rate == 0 \
		     , "Injection rate is non-zero for node outside board"
	else:
		if global_injection_rate is None:
			global_injection_rate = injection_rate
		assert global_injection_rate == injection_rate \
		     , "Injection rate is not the same accross the whole system"
	
	# Check the timeout is consistent throughout
	timeout = data_chunk[num_chips + 3]
	if global_timeout is None:
		global_timeout = timeout
	assert global_timeout == timeout \
	     , "Timeout is not the same globally"
	
	# Check that the sampling period is consistent throughout
	sample_period = data_chunk[num_chips + 4]
	if global_sample_period is None:
		global_sample_period = sample_period
	assert sample_period == global_sample_period \
	     , "Sample period is not the same globally."


#print "Config Summary:"
#print "  System size: %dx%d"%(system_size, system_size)
#print "  Sample period: %d"%(global_sample_period)
#print "  Router timeout: %d"%(global_timeout)
#print "  Injection rate: %d"%(global_injection_rate)
#print "  Consumption rate: %d"%(global_consumption_rate)
