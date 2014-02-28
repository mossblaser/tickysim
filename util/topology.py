#!/usr/bin/env python

"""
Utilities for working with the hexagonal toroidal-mesh topology used in
SpiNNaker.

Contains:

  * Constants/Functions for directions in the space that the chips sit.
  * Constants for identifying edges of a hexagon of chips (uses same functions
    as directions)
  * Functions for addressing in a hexagonal world.
  * Functions for generating arrangements of hexagons.

This uses the addressing scheme suggested in

Addressing and Routing in Hexagonal Networks with Applications for Tracking
Mobile Users and Connection Rerouting in Cellular Networks by Nocetti et. al.

With the "z" dimension omitted (and assumed to be zero). X points from
left-to-right, Y points from bottom-to-top and Z points from
top-right-to-bottom-left.
"""

################################################################################
# Directions
################################################################################

EAST       = 0
NORTH_EAST = 1
NORTH      = 2
WEST       = 3
SOUTH_WEST = 4
SOUTH      = 5


def next_ccw(direction):
	"""
	Returns the next direction counter-clockwise from the given direction.
	"""
	return (direction+1)%6


def next_cw(direction):
	"""
	Returns the next direction counter-clockwise from the given direction.
	"""
	return (direction-1)%6


def opposite(direction):
	"""
	Returns the opposite direction
	"""
	return (direction+3)%6


################################################################################
# Edges of a hexagon of chips
################################################################################

EDGE_TOP         = 0
EDGE_TOP_LEFT    = 1
EDGE_BOTTOM_LEFT = 2
EDGE_BOTTOM      = 3
EDGE_BOTTOM_RIGHT= 4
EDGE_TOP_RIGHT   = 5


################################################################################
# Coordinates in hexagon world :)
################################################################################

def add_direction(vector, direction):
	"""
	Returns the vector moved one unit in the given direction.
	"""
	add = {
		EAST:       ( 1, 0, 0),
		WEST:       (-1, 0, 0),
		NORTH:      ( 0, 1, 0),
		SOUTH:      ( 0,-1, 0),
		NORTH_EAST: ( 0, 0,-1),
		SOUTH_WEST: ( 0, 0, 1),
	}
	
	return tuple(v + a for (v,a) in zip(vector, add[direction]))

def manhattan(vector):
	"""
	Calculate the Manhattan distance required to traverse the given vector.
	"""
	return sum(map(abs, vector))


def median_element(values):
	"""
	Returns the value of the median element of the set.
	"""
	return sorted(values)[len(values)/2]


def to_shortest_path(vector):
	"""
	Converts a vector into the shortest-path variation.
	
	A shortest path has at least one dimension equal to zero and the remaining two
	dimensions have opposite signs (or are zero).
	"""
	assert(len(vector) == 3)
	
	# The vector (1,1,1) has distance zero so this can be added or subtracted
	# freely without effect on the destination reached. As a result, simply
	# subtract the median value from all dimensions to yield the shortest path.
	median = median_element(vector)
	return tuple(v - median for v in vector)


def to_xy(vector):
	"""
	Takes a 3D vector and returns the equivalent 2D version.
	"""
	return (vector[0] - vector[2], vector[1] - vector[2])


def get_path(src, dst, bounds = None):
	"""
	Gets the shortest path from src to dst.
	
	If bounds is given it must be a 2-tuple specifying the (x,y) dimensions of the
	mesh size. The path will then be allowed to 'wrap-around', otherwise it will
	not.
	"""
	assert(len(src) == len(dst) == 3)
	assert(bounds is None or len(bounds) == 2)
	
	src = to_xy(src)
	dst = to_xy(dst)
	
	# If bounded, re-centre the world around the source
	if bounds is not None:
		delta = None
		# This is a terrible hack. Re-centre the world around the bottom left,
		# center and top-right in order to find the /actual/ shortest path. I and
		# a number of other very helpful people have spent literally hours and hours
		# on trying to solve this problem elegantly before I gave up and did this...
		for centre in (0.0, 0.5, 1):
			new_dst = ( ((dst[0] - src[0]) + int(bounds[0]*centre))   % bounds[0]
			          , ((dst[1] - src[1]) + int(bounds[1]*centre))   % bounds[1]
			          , 0
			          )
			new_src = ( int(bounds[0]*centre)
			          , int(bounds[1]*centre)
			          , 0
			          )
			new_delta = to_shortest_path(zero_pad(tuple(d-s for (s,d) in zip(new_src, new_dst))))
			if delta is None or manhattan(new_delta) < manhattan(delta):
				delta = new_delta
		
	else:
		# The path is simply a delta of the source and destination
		delta = to_shortest_path(zero_pad(tuple(d-s for (s,d) in zip(src, dst))))
	
	# Return the shortest path to the given point
	return delta


def zero_pad(vector, length = 3):
	"""
	Zero pad a vector to the required length.
	"""
	return tuple((list(vector) + ([0]*length))[:length])



################################################################################
# Hexagon Generation
################################################################################

def hexagon(layers = 4):
	"""
	Generator which produces a list of (x,y) tuples which produce a hexagon of the
	given number of layers.
	
	Try me::
	
		points = set(hexagon(4))
		for y in range(min(y for (x,y) in points), max(y for (x,y) in points) + 1)[::-1]:
			for x in range(min(x for (x,y) in points), max(x for (x,y) in points) + 1):
				if (x,y) in points:
					print "#",
				else:
					print " ",
			print
	"""
	
	X,Y,Z = 0,1,2
	
	next_position = [0,0,0]
	
	for n in range(layers):
		for _ in range(n):
			yield to_xy(next_position)
			next_position[Y] -= 1
		
		for _ in range(n):
			yield to_xy(next_position)
			next_position[Z] += 1
		
		for _ in range(n+1):
			yield to_xy(next_position)
			next_position[X] -= 1
		
		for _ in range(n):
			yield to_xy(next_position)
			next_position[Y] += 1
		
		for _ in range(n+1):
			yield to_xy(next_position)
			next_position[Z] -= 1
		
		for _ in range(n+1):
			yield to_xy(next_position)
			next_position[X] += 1


def hexagon_edge_link(edge, num, layers=4):
	"""
	Given an edge (EDGE_*) and an num (0-(layers*2 - 1)) returns the ((x,y),
	direction) pair of the relevant link.
	
	The "edges" of a layers=4 hexagon are given below
	           11111111
	         00# # # #22
	       00# # # # #22   0 = EDGE_TOP_LEFT
	     00# # # # # #22   1 = EDGE_TOP
	   00# # # # # # #22   2 = EDGE_TOP_RIGHT
	  5# # # # 0 # # #3    3 = EDGE_BOTTOM_RIGHT
	 55# # # # # # #33     4 = EDGE_BOTTOM
	 55# # # # # #33       5 = EDGE_BOTTOM_LEFT
	 55# # # # #33
	  5444444443
	
	XXX: This function is implemented in a way which works but which may not be
	especially logical, unfortunately I don't understand how these edges are
	defined well enough to do better...
	"""
	assert(0 <= num < layers*2)
	
	# The coordinate to start iterating over the links
	edge_start = {
		EDGE_TOP         : (0,           layers    ),
		EDGE_TOP_LEFT    : (-layers,     0         ),
		EDGE_BOTTOM_LEFT : (-layers,    -layers + 1),
		EDGE_BOTTOM      : (-layers,    -layers + 1),
		EDGE_BOTTOM_RIGHT: (0,          -layers + 1),
		EDGE_TOP_RIGHT   : (layers - 1, 0),
	}[edge]
	
	# The direction in which consecutive nodes on the edge go
	edge_direction = {
		EDGE_TOP         : ( 1, 0),
		EDGE_TOP_LEFT    : ( 1, 1),
		EDGE_BOTTOM_LEFT : ( 0, 1),
		EDGE_BOTTOM      : ( 1, 0),
		EDGE_BOTTOM_RIGHT: ( 1, 1),
		EDGE_TOP_RIGHT   : ( 0, 1),
	}[edge]
	
	# The pair of directions of links which this edge exposes.
	edge_links = {
		EDGE_TOP         : (NORTH,      NORTH_EAST),
		EDGE_TOP_LEFT    : (NORTH,      WEST),
		EDGE_BOTTOM_LEFT : (SOUTH_WEST, WEST),
		EDGE_BOTTOM      : (SOUTH,      SOUTH_WEST), # Opposite links for the opposite edge
		EDGE_BOTTOM_RIGHT: (SOUTH,      EAST),
		EDGE_TOP_RIGHT   : (NORTH_EAST, EAST),
	}[edge]*layers
	
	# Does the first (and last) node in this set of links only have one link exposed?
	first_node_single = {
		EDGE_TOP         : False,
		EDGE_TOP_LEFT    : True,
		EDGE_BOTTOM_LEFT : False,
		EDGE_BOTTOM      : True,
		EDGE_BOTTOM_RIGHT: False,
		EDGE_TOP_RIGHT   : True,
	}[edge]
	
	# Get the offset into the edges nodes
	node_offset = (num+first_node_single) / 2
	
	# Get the address of the node at that offset
	node = tuple(s+(o*node_offset) for (s,o) in zip(edge_start, edge_direction))
	
	# The link direction from the target node
	link_direction = edge_links[num%2]
	
	return node, link_direction
