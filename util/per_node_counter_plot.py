#!/usr/bin/env python

r"""
Generate LaTeX files containing diagrams of per-node counter information.

Usage::

	python per_node_counter_plot.py path/to/per_node_counters.dat \
	                                node_scale \
	                                xcol:ycol \
	                                list:of:columns:to:plot

node_scale is the size of the hexagons

xcol, ycol are the column indexes of the x and y coordinates of the nodes in the
input file.

the list of columns to plot should be a colon-seperated list of field indexes to
show in the figure.
"""

def generate_tikz(scale, node_labels):
	"""
	Generate a minimal LaTeX file containing a set of hexagons with the given
	labels. node_labels should be a dict {(x,y):"label",...}.
	"""
	
	hexagons = "\n".join("\myhex{%d,%d,0}{%s}"%(x,y,s)
	                     for ((x,y),s) in node_labels.iteritems())
	
	return r"""
	%% Only build the bare minimum
	%%\documentclass{minimal}
	\documentclass[border=0pt,12pt]{standalone}
	
	\usepackage{amsmath}
	\usepackage{amssymb}
	
	\usepackage{graphicx}
	
	\usepackage{ifthen}
	
	\usepackage[outline]{contour}
	\contourlength{1.5pt}
	
	\usepackage{tikz}
	\usepackage{tikz3d}
	\usetikzlibrary{ hexagon
	               , calc
	               , backgrounds
	               , positioning
	               , decorations.pathreplacing
	               , decorations.markings
	               , arrows
	               , positioning
	               , automata
	               , shadows
	               , fit
	               , shapes
	               , arrows
	               }
	
	\begin{document}
		\begin{tikzpicture}[every text node part/.style={align=center}]
			\newcommand{\myhex}[2]{
				\node (myhex node) [draw,hexagon,inner sep=0,minimum size=%(scale)fcm] at (#1) {};
				\node at (myhex node) {#2};
			}
			
			\begin{scope}[hexagonXYZ,scale=%(scale)f]
				%(hexagons)s
			\end{scope}
		\end{tikzpicture}
	\end{document}
	"""%{
		"scale": scale,
		"hexagons" : hexagons,
	}


if __name__=="__main__":
	import sys
	
	data_filename = sys.argv[1]
	scale         = float(sys.argv[2])
	xcol,ycol     = map(int, sys.argv[3].split(":"))
	data_columns  = map(int, sys.argv[4].split(":"))
	
	data_raw = open(data_filename,"r").read().strip().split("\n")
	data_header = data_raw[0].split("\t")
	data_values = [s.split("\t") for s in data_raw[1:]]
	
	node_labels = {}
	
	for row in data_values:
		label = r"\\".join("%s: %s"%(data_header[col-1].replace("_",r"\textunderscore{}"), row[col-1])
		                   for col in data_columns
		                  )
		
		node_labels[(int(row[xcol-1]), int(row[ycol-1]))] = label
	
	print generate_tikz(scale, node_labels)
