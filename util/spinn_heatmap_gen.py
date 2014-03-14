#!/usr/bin/env python

"""
Reads in TSVs of pages of SpiNNaker's heatmap results and produces a TSV formatted
as the heatmap generator would enjoy.

Usage:
spinnaker_heatmap_gen.py 12 12 some_stuff_cyclic.tsv some_stuff_tornado.tsv some_stuff_transpose.tsv some_stuff_complement.tsv
"""


import sys

from collections import defaultdict


def extract_runs(raw_data):
	"""
	Given a TSV file as a string, returns a dictionary::
		
		{inj_period: [(injected, extracted, dumped, routed)], ...}
	"""
	
	out = defaultdict(list)
	
	rows = [r.split("\t") for r in raw_data.split("\n")[10:]]
	for row in rows:
		# Skip rows which are either the injection rate or headings (i.e. those
		# whose first column isn't the chip ID integer).
		try:
			int(row[0])
		except ValueError:
			continue
		
		for run_cols_start in range(1, len(row), 4):
			inj_period = int(16.0/float(rows[0][run_cols_start]))
			out[inj_period].append(map(int,row[run_cols_start:run_cols_start+4]))
	
	return out


def render_heatmap_file(patterns, width, height, link_latency=16):
	"""
	Given a dictionary of the form {pattern_name: runs} where runs is a dictionary
	as produced by extract_runs(), make a TSV file with the following columns:
	
		chip_x, chip_y, injected, extracted, dumped, routed, inj_rate, inj_period, pattern
	
	Also takes the width and height of the system.
	"""
	
	out = "chip_x\tchip_y\tinjected\textracted\tdumped\trouted\tinj_rate\tinj_period\tpattern\n"
	
	for pattern_name, runs in patterns.iteritems():
		for inj_period, run in runs.iteritems():
			for core_id, (injected,extracted,dumped,routed) in enumerate(run):
				out += "%d\t%d\t%d\t%d\t%d\t%d\t%f\t%d\t%s\n"%(
					# Get the x and y coordinate of the chip
					core_id/width, core_id%width,
					# The results
					injected,extracted,dumped,routed,
					# The injection rate
					float(link_latency)/inj_period,
					inj_period,
					pattern_name
				)
	
	return out


if __name__=="__main__":
	width, height = map(int, sys.argv[1:3])
	
	patterns = {}
	for filename in sys.argv[3:]:
		pattern_name = filename.rpartition(".")[0].rpartition("_")[2]
		runs = extract_runs(open(filename,"r").read())
		patterns[pattern_name] = runs
	
	print render_heatmap_file(patterns, width,height)
