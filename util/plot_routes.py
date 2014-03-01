#!/usr/bin/env python

"""
Plot all routes which will be traversed through a system for a given size and
traffic pattern. Very quick hack...
"""



import sys
import random

import topology

PATTERN = sys.argv[1]
WIDTH  = int(sys.argv[2])
HEIGHT = int(sys.argv[3])

DIMENSION_ORDER = [0,1,2]


def seg_intersect(l1, l2):
	"""
	Find the intersection between two lines
	"""
	return (None, None)


def line_offset(src):
	"""
	Offset a line depending on where the line starts.
	"""
	GAP = 0.1
	# Note swapped axes
	return [ -0.5 + GAP + ((float(src[1]) / float(WIDTH))  * (1.0 - (GAP*2)))
	       , 0.5 - (GAP + ((float(src[0]) / float(HEIGHT)) * (1.0 - (GAP*2))))
	       ]


def print_route(src, dst):
	"""
	Print the gnuplot line segments for a route between the two defined points.
	"""
	
	points = []
	
	route = list(topology.get_path(src, dst, (WIDTH,HEIGHT)))
	
	offset = line_offset(src)
	def print_point(point):
		points.append(tuple(p+o for p,o in zip(point,offset)))
	def print_gap():
		points.append(None)
	
	# Print the start of the line
	cur_pos = list(src)
	print_point(cur_pos)
	
	# Print subsequent line segments
	for dimension in DIMENSION_ORDER:
		# Used to change the offset half-way
		completed_first_part = route[dimension] != 0
		
		while route[dimension] != 0:
			if route[dimension] < 0:
				if dimension != 2:
					cur_pos[dimension] -= 1
				else:
					cur_pos[0] += 1
					cur_pos[1] += 1
				route[dimension] += 1
			elif route[dimension] > 0:
				if dimension != 2:
					cur_pos[dimension] += 1
				else:
					cur_pos[0] -= 1
					cur_pos[1] -= 1
				route[dimension] -= 1
			print_point(cur_pos)
			
			# Have we gone off the edge? Break the line and wrap around.
			wrap = [0,0]
			extra_step = [0,0]
			
			if cur_pos[0] < 0:
				wrap[0] += WIDTH + 1
				extra_step[0] -= 1
			if cur_pos[0] >= WIDTH:
				wrap[0] -= WIDTH + 1
				extra_step[0] += 1
			
			if cur_pos[1] < 0:
				wrap[1] += HEIGHT + 1
				extra_step[1] -= 1
			if cur_pos[1] >= HEIGHT:
				wrap[1] -= HEIGHT + 1
				extra_step[1] += 1
			
			if wrap:
				print_gap()
				cur_pos[0] += wrap[0]
				cur_pos[1] += wrap[1]
				print_point(cur_pos)
				cur_pos[0] += extra_step[0]
				cur_pos[1] += extra_step[1]
				print_point(cur_pos)
		
		# Change the offset at the inflection point
		if completed_first_part and route:
			offset = line_offset(dst)
			#print_gap()
			print_point(cur_pos)
	print_gap()
	
	for point in points:
		if point is None:
			print ""
		else:
			print "%f\t%f"%(point[0], point[1])


def cyclic(start):
	for x in range(WIDTH):
		for y in range(HEIGHT):
			yield (x,y)


def tornado(start):
	return [ ((WIDTH/2 + start[0])%WIDTH, start[1]) ]


def transpose(start):
	return [ (start[1], start[0]) ]


def complement(start):
	return [ (WIDTH - start[0] - 1, HEIGHT - start[1] - 1) ]


PATTERNS = {
	"cyclic": cyclic,
	"tornado": tornado,
	"transpose": transpose,
	"complement": complement,
}

if PATTERN in PATTERNS:
	for x in range(WIDTH):
		for y in range(HEIGHT):
			for target in PATTERNS[PATTERN]((x,y)):
				print_route((x,y), target)
else:
	print "-2\t-2"
	print "-2\t-2"
