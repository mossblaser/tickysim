OUT_DIR="heatmaps/"

# SIMULATOR FILE                                 PATTERN    INJ_INTERVAL  DATA_COL  DATA_NAME  X_COL  Y_COL  PATTERN_COL  INJ_INTERVAL_COL
EXPERIMENTS="$(cat <<EOF
Tickysim    results/ed8f5e8_per_node_counters.dat  cyclic     320           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  cyclic      53           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  cyclic      53            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  cyclic      32            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  complement 320           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  complement  53           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  complement  53            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  complement  32            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  transpose  320           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  transpose   53           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  transpose   53            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  transpose   32            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  tornado    320           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  tornado     53           10        Routed     5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  tornado     53            9        Dropped    5      6      3            4
Tickysim    results/ed8f5e8_per_node_counters.dat  tornado     32            9        Dropped    5      6      3            4
SpiNNaker   results/spinnaker_results.csv          cyclic     320            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          cyclic      53            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          cyclic      53            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          cyclic      32            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          complement 320            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          complement  53            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          complement  53            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          complement  32            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          transpose  320            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          transpose   53            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          transpose   53            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          transpose   32            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          tornado    320            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          tornado     53            6        Routed     1      2      9            8
SpiNNaker   results/spinnaker_results.csv          tornado     53            5        Dropped    1      2      9            8
SpiNNaker   results/spinnaker_results.csv          tornado     32            5        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             cyclic     120            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             cyclic      53            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             cyclic      53            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             cyclic      32            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             complement 120            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             complement  53            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             complement  53            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             complement  32            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             transpose  120            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             transpose   53            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             transpose   53            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             transpose   32            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             tornado    120            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             tornado     53            5        Routed     1      2      9            8
FPGA        results/mohsen_results.csv             tornado     53            3        Dropped    1      2      9            8
FPGA        results/mohsen_results.csv             tornado     32            3        Dropped    1      2      9            8
EOF
)"

echo "$EXPERIMENTS" | while read SIMULATOR FILE PATTERN INJ_INTERVAL DATA_COL DATA_NAME X_COL Y_COL PATTERN_COL INJ_INTERVAL_COL; do
	INJ_LOAD="$(python -c "print \"%0.2f\"%(16.0/$INJ_INTERVAL)")"
	
	OUTFILE="${OUT_DIR}/${SIMULATOR}_${PATTERN}_${INJ_LOAD}_${DATA_NAME}.png"
	
	mkdir -p "$(dirname "$OUTFILE")"
	
	gnuplot -e "SIMULATOR=\"${SIMULATOR}\";" \
	        -e "FILE=\"${FILE}\";" \
	        -e "PATTERN=\"${PATTERN}\";" \
	        -e "INJ_INTERVAL=${INJ_INTERVAL};" \
	        -e "DATA_COL=${DATA_COL};" \
	        -e "DATA_NAME=\"${DATA_NAME}\";" \
	        -e "X_COL=${X_COL};" \
	        -e "Y_COL=${Y_COL};" \
	        -e "PATTERN_COL=${PATTERN_COL};" \
	        -e "INJ_INTERVAL_COL=${INJ_INTERVAL_COL};" \
	        $(dirname "$0")/heatmap_plot.gnuplot \
	  > "$OUTFILE"
done
