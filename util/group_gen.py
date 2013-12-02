#!/usr/bin/env python

"""
Calculate link periods which achieve certain loads.

Usage:

	python group_gen.py link_latency [start_latency, end_latency, latency_step] ...

Produces list of periods which achieve a normalised load of values between
start_latency and end_latency in increments of latency_step. Multiple ranges can
be supplied and will be merged together.
"""


def frange(start, end=None, inc=None):
	"""
	A range function, that does accept float increments...
	
	Taken from http://code.activestate.com/recipes/66472/
	"""
	
	if end == None:
		end = start + 0.0
		start = 0.0
	
	if inc == None:
		inc = 1.0
	
	L = []
	while 1:
		next = start + len(L) * inc
		if inc > 0 and next >= end:
			break
		elif inc < 0 and next <= end:
			break
		L.append(next)
	
	return L


def load_to_period(load, link_latency):
	"""
	Given a specified load (e.g. 0.1), and the latency of sending a packet down a
	link, computes the period between packets which will achieve that proprtion of
	traffic being sent into the link.
	"""
	return int(round(link_latency/load))


if __name__=="__main__":
	import sys
	
	link_latency = float(sys.argv[1])
	
	experiments = {}
	
	it = iter(map(float, sys.argv[2:]))
	for start, end, inc in zip(it,it,it):
		for load in frange(start,end,inc):
			experiments[load_to_period(load, link_latency)] = load
	
	for interval, load in sorted( experiments.iteritems()
	                            , key=(lambda (i,l):l)
	                            ):
		print "(%3d) # Normalised load = %0.2f"%(interval, load)
