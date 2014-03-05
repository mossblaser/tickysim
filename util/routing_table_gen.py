#!/usr/bin/env python

"""
Generate routing tables for other models.
"""

import topology

class RoutingTableGen(object):
	
	def __init__(self, system_size, system_shape = "torus"):
		"""
		system_size is a tuple (w,h) stating the size of the system
		
		system_shape is a string. "torus" indicates a rectangular torus based
		system, "board" indicates a radius 4 SpiNNaker board (e.g. spinn-4/spinn-5).
		"""
		
		assert system_shape in ("torus", "board") \
		     , "Unknown system shape!"
		
		assert not (system_shape == "board" and (system_size[0] < 8 or system_size[1] < 8)) \
		     , "Board-shaped systems must be at least 8x8 in size."
		
		self.system_size = system_size
		self.system_shape = system_shape
	
	
	def is_in_system(self, pos):
		"""
		Tests whether a given (x,y) position is within the system.
		"""
		
		# Test if within system size
		if not (0 <= pos[0] < self.system_size[0]
		     or 0 <= pos[1] < self.system_size[1]):
			return False
		
		# Test whether within confines of the system's shape
		if self.system_shape == "torus":
			# All chips are active in a torus
			return True
		elif self.system_shape == "board":
			# Only chips on the hexagonal center are active in a board
			return (pos[0]-4, pos[1]-3) in list(topology.hexagon())
	
	
	def next_chip(self, pos):
		"""
		Return the next chip going left-to-right, bottom-to-top in the system.
		"""
		x = pos[0]+1
		y = pos[1]
		
		if x >= self.system_size[0]:
			x = 0
			y += 1
		
		if y >= self.system_size[1]:
			y = 0
		
		if self.is_in_system((x,y)):
			return (x,y)
		else:
			return self.next_chip((x,y))
	
	
	def get_route(self, source, destination):
		"""
		Given a source node and a destination node as (x,y) tuples, returns the
		direction which the packet should be routed in from the source node.
		"""
		
		vector = (0,0,0)
		
		# Work out the vector to travel along for the current system
		if self.system_shape == "torus":
			vector = topology.get_path(source, destination, self.system_size)
		elif self.system_shape == "board":
			vector = topology.to_shortest_path(( destination[0]-source[0]
			                                   , destination[1]-source[1]
			                                   , 0
			                                   ))
		
		# Return the relevant direction from the source
		if vector == (0,0,0):
			return topology.LOCAL
		elif vector[0] < 0:
			return topology.WEST
		elif vector[0] > 0:
			return topology.EAST
		elif vector[1] < 0:
			return topology.SOUTH
		elif vector[1] > 0:
			return topology.NORTH
		elif vector[2] < 0:
			return topology.NORTH_EAST
		elif vector[2] > 0:
			return topology.SOUTH_WEST



class FPGATableGen(RoutingTableGen):
	
	def __init__( self
	            , system_size = (8,8)
	            , system_shape = "board"
	            , traffic_pattern = "cyclic"
	            , injection_rate = 16
	            , consumption_delay = 10
	            , router_timeout = 50
	            , sample_period = 60000
	            ):
		RoutingTableGen.__init__(self, system_size, system_shape)
		
		self.traffic_pattern   = traffic_pattern
		self.injection_rate    = injection_rate
		self.consumption_delay = consumption_delay
		self.router_timeout    = router_timeout
		self.sample_period     = sample_period
	
	
	def pos_to_key(self, pos):
		"""
		Return the routing key for a given position in the system
		"""
		assert pos[0] < 1<<4, "X-Position too large to represent in key."
		assert pos[1] < 1<<4, "Y-Position too large to represent in key."
		
		return pos[0]<<4 | pos[1]
	
	
	def generate_routing_table(self, pos):
		"""
		Generate the routing table for a single node at the given position. Returns
		table in the form of a list of integers.
		"""
		if not self.is_in_system(pos):
			# Empty table for nodes not in the system
			return [0xFF00]*(self.system_size[0] * self.system_size[1])
		else:
			table = []
			skipped = 0
			for y in range(self.system_size[1]):
				for x in range(self.system_size[0]):
					if not self.is_in_system((x,y)):
						skipped += 1
						continue
					
					dest = self.pos_to_key((x,y))
					
					direction = 1<<self.get_route(pos, (x, y))
					assert direction < 1<<7, "Invalid direction found by get_route"
					
					table.append(dest<<8 | direction)
			table += [0xFF00]*skipped
			return table
	
	
	def generate_injection_pattern(self, pos):
		"""
		Given the position of a particular chip, return the first two 16-bit words
		of the configuration block defining the first traffic destination, node
		position and number of destinations.
		"""
		
		if not self.is_in_system(pos):
			dest = (0xF, 0xF)
			num_dests = 0
		elif self.traffic_pattern == "cyclic":
			dest = self.next_chip(pos)
			num_dests = 0
			# Every active and non-self chip
			for y in range(self.system_size[1]):
				for x in range(self.system_size[0]):
					if self.is_in_system((x,y)) and (x,y) != pos:
						num_dests += 1
		elif self.traffic_pattern == "complement":
			dest = (self.system_size[0]- pos[0] - 1, self.system_size[1] - pos[1] - 1)
			num_dests = 1
		elif self.traffic_pattern == "transpose":
			dest = (pos[1], pos[0])
			num_dests = 1
		elif self.traffic_pattern == "tornado":
			dest = ((self.system_size[0]/2 + pos[0])%self.system_size[0], pos[1])
			num_dests = 1
		else:
			assert False, "Unknown traffic pattern!"
		
		# Sanity check the pattern
		assert self.is_in_system(dest) or not self.is_in_system(pos) \
		     , "Traffic pattern uses chips not in the system!"
		
		return [ self.pos_to_key(dest)<<8 | self.pos_to_key(pos)
		       , 0x00<<8                  | num_dests
		       ]
	
	
	def generate_table(self):
		"""
		Returns a string containing the routing/configuration tables for the FPGA
		model with the given experimental configuration.
		"""
		
		# Check all fields are within representable ranges
		assert self.injection_rate    < 1<<10, "Injection rate too large for field"
		assert self.consumption_delay < 1<<6,  "Consumption delay too large for field"
		assert self.router_timeout    < 1<<16, "Router timeout too large for field"
		assert self.sample_period     < 1<<16, "Sample period too large for field"
		
		out = []
		
		# Add table entries for each chip in the system
		for y in range(self.system_size[1]):
			for x in range(self.system_size[0]):
				out += self.generate_routing_table((x,y))
				out += self.generate_injection_pattern((x,y))
				out += [ self.consumption_delay<<10 | self.injection_rate
				       , self.router_timeout
				       , self.sample_period
				       ]
		
		# Add terminator
		out.append(0xBBBB)
		
		# Format for table file
		return "\n".join("%04X"%d for d in out)


if __name__=="__main__":
	import sys
	
	# Print help message
	if len(sys.argv) == 1:
		print "Usage:"
		print "  %s fpga [system_size] [system_shape] [traffic_pattern] [injection_rate] [consumption_delay] [router_timeout] [sample_period]"%sys.argv[0]
		print "Example:"
		print "  %s fpga 8,8 board cyclic 16 10 50 60000"%sys.argv[0]
		sys.exit(0)
	
	if sys.argv[1] == "fpga":
		g = FPGATableGen( system_size       = map(int,sys.argv[2].split(","))
		                , system_shape      = sys.argv[3]
		                , traffic_pattern   = sys.argv[4]
		                , injection_rate    = int(sys.argv[5])
		                , consumption_delay = int(sys.argv[6])
		                , router_timeout    = int(sys.argv[7])
		                , sample_period     = int(sys.argv[8])
		                )
		print g.generate_table()
