#!/usr/bin/env python

import sys

from topology import *

# Get the outline of a hexagon
lines = []
for edge in range(6):
	for num in range(8):
		pos, direction = hexagon_edge_link(edge, num)
		
		pos = (pos[0]+4, pos[1]+3)
		
		if direction == NORTH:
			lines.append(( (pos[0]-0.5, pos[1]+0.5)
			             , (pos[0]+0.5, pos[1]+0.5) ))
		elif direction == SOUTH:
			lines.append(( (pos[0]-0.5, pos[1]-0.5)
			             , (pos[0]+0.5, pos[1]-0.5) ))
		elif direction == WEST:
			lines.append(( (pos[0]-0.5, pos[1]+0.5)
			             , (pos[0]-0.5, pos[1]-0.5) ))
		elif direction == EAST:
			lines.append(( (pos[0]+0.5, pos[1]+0.5)
			             , (pos[0]+0.5, pos[1]-0.5) ))

width  = int(sys.argv[1])
height = int(sys.argv[2])
full_set = []

for x in range(width):
	for y in range(height):
		for z in range(3):
			x_offset = 4*(x*3 + z)
			y_offset = 4*(y*3 + (3-z)%3)
			
			for line in lines:
				full_set.append(tuple(
					(x+x_offset,y+y_offset) for (x,y) in line
				))

cropped_set = []
for line in set(full_set):
	if     all(x < ((12*width)  + 0.5) for (x,y) in line) \
	   and all(y < ((12*height) + 0.5) for (x,y) in line):
		cropped_set.append(line)
cropped_set.append(((-0.5, -0.5), (12*width - 0.5, -0.5)))
cropped_set.append(((-0.5, -0.5), (-0.5, 12*width - 0.5)))
cropped_set.append(((12*width - 0.5, -0.5), (12*width - 0.5, 12*width - 0.5)))
cropped_set.append(((-0.5, 12*width - 0.5), (12*width - 0.5, 12*width - 0.5)))

for start, stop in set(cropped_set):
	print "\t".join(map(str, start))
	print "\t".join(map(str, stop))
	print
