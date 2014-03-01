#!/usr/bin/env python

"""
Take a heatmap data file and pad out the data with zeros to make it suitable for
gnuplot.
"""

import sys

x_col = int(sys.argv[1]) - 1
y_col = int(sys.argv[2]) - 1


data = map(str.split, sys.stdin.read().strip().split("\n"))

blocks = []
cur_block = []

# Split up on changes in y
last_y = None
for line in data:
	y = line[y_col]
	if y != last_y:
		if last_y is not None:
			blocks.append(cur_block)
		cur_block = []
		last_y = y
	cur_block.append(line)
blocks.append(cur_block)


# Work out the maximum x dimension of the heatmap
max_x = max(int(line[x_col]) for line in data)


# Pad out each block's X
for block in blocks:
	block_y = block[0][y_col]
	
	# Pad front of each block
	for x in range(0, int(block[0][x_col])):
		new_row = ["0"]*len(block[0])
		new_row[x_col] = str(x)
		new_row[y_col] = block_y
		block.insert(x, new_row)
	
	# Pad end of each block
	for x in range(int(block[-1][x_col])+1, max_x+1):
		new_row = ["0"]*len(block[0])
		new_row[x_col] = str(x)
		new_row[y_col] = block_y
		block.append(new_row)

print "\n\n".join("\n".join("\t".join(line) for line in block) for block in blocks)
