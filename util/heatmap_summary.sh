#!/bin/bash

# A very quick hack of a script which will produce a HTML table of the heatmaps
# generated by heatmap_plot.sh.
# Usage:
#  heatmap_summary.sh [heatmap_dir] [arguments to grep filtering simulator names...]


HEATMAP_DIR="$1"
shift

SIMULATORS=($(cd "$HEATMAP_DIR"; ls *.png | cut -f1 -d_ | grep "$@" | sort | uniq))
EXPERIMENTS=($(cd "$HEATMAP_DIR"; ls *.png | cut -f2-4 -d_ | sort -n | uniq))

echo "<!-- I'm sorry you had to see this... -->"

echo '<ul>'
for EXPERIMENT in ${EXPERIMENTS[@]}; do
	# Does this experiment have results for all?
	RESULTS_FOR_ALL=yes
	for SIMULATOR in ${SIMULATORS[@]}; do
		FILENAME="$HEATMAP_DIR/${SIMULATOR}_$EXPERIMENT"
		if [ ! -f "$FILENAME" ]; then
			RESULTS_FOR_ALL=no
		fi
	done
	
	# Skip when we don't have results for everyone
	if [ "$RESULTS_FOR_ALL" == "no" ]; then
		continue
	fi
	
	echo "  <li><a href=\"#$EXPERIMENT\">$EXPERIMENT</a></li>"
done
echo '</ul>'

echo '<table style="width: 100%;">'
echo '  <thead style="font-weight: bold;"><tr>'
echo '    <td>Experiment</td>'
	for SIMULATOR in ${SIMULATORS[@]}; do
		printf "    <td>$SIMULATOR</td>"
	done
echo '  </tr></thead>'

echo '  <tbody>' 

for EXPERIMENT in ${EXPERIMENTS[@]}; do
	# Does this experiment have results for all?
	RESULTS_FOR_ALL=yes
	for SIMULATOR in ${SIMULATORS[@]}; do
		FILENAME="$HEATMAP_DIR/${SIMULATOR}_$EXPERIMENT"
		if [ ! -f "$FILENAME" ]; then
			RESULTS_FOR_ALL=no
		fi
	done
	
	# Skip when we don't have results for everyone
	if [ "$RESULTS_FOR_ALL" == "no" ]; then
		continue
	fi
	
	echo '  <tr>'
	printf "    <td><a id="$EXPERIMENT"></a>*$EXPERIMENT</td>"
		for SIMULATOR in ${SIMULATORS[@]}; do
			FILENAME="$HEATMAP_DIR/${SIMULATOR}_$EXPERIMENT"
			if [ -f "$FILENAME" ]; then
				echo "    <td><img style=\"width:100%;\" src=\"data:image/png;base64,$(base64 -w0 "$FILENAME")\" title=\"$SIMULATOR\"></td>"
			else
				echo '    <td style="text-align: center;">No Data</td>'
			fi
		done
	echo '  </tr>'
done

echo '  </tbody>' 
echo '</table>' 
