OUT_DIR="heatmaps"
DRAW_ROUTES_OUT_DIR="heatmaps_with_routes"

# SIMULATOR W  H  FILE                                   PATTERN       INJ_INTERVAL  DATA_COL  DATA_NAME  X_COL  Y_COL  PATTERN_COL  INJ_INTERVAL_COL
EXPERIMENTS="$(cat <<EOF
Tickysim    12 12 results/e74e47a_per_node_counters.dat  cyclic        320           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  cyclic         53           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  cyclic         53            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  cyclic         32            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  complement    320           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  complement     53           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  complement     53            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  complement     32            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  transpose     320           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  transpose      53           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  transpose      53            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  transpose      32            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  tornado       320           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  tornado        53           10        Routed     5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  tornado        53            9        Dropped    5      6      3            4
Tickysim    12 12 results/e74e47a_per_node_counters.dat  tornado        32            9        Dropped    5      6      3            4
Tickysim    12 12 results/89dc372_per_node_counters.dat  singlesource   16           10        Routed     5      6      3            4
Tickysim    12 12 results/89dc372_per_node_counters.dat  singlesource   32           10        Routed     5      6      3            4
Tickysim    12 12 results/89dc372_per_node_counters.dat  singlesource  120           10        Routed     5      6      3            4
SpiNNaker   12 12 results/spinnaker_results.csv          cyclic        320            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          cyclic         53            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          cyclic         53            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          cyclic         32            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          complement    320            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          complement     53            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          complement     53            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          complement     32            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          transpose     320            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          transpose      53            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          transpose      53            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          transpose      32            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          tornado       320            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          tornado        53            6        Routed     1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          tornado        53            5        Dropped    1      2      9            8
SpiNNaker   12 12 results/spinnaker_results.csv          tornado        32            5        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             cyclic        120            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             cyclic         53            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             cyclic         53            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             cyclic         32            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             cyclic         16            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             complement    120            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             complement     53            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             complement     53            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             complement     32            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             transpose     120            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             transpose      53            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             transpose      53            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             transpose      32            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             tornado       120            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             tornado        53            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             tornado        53            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             tornado        32            3        Dropped    1      2      9            8
FPGA        12 12 results/mohsen_results.csv             singlesource   16            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             singlesource   32            5        Routed     1      2      9            8
FPGA        12 12 results/mohsen_results.csv             singlesource  120            5        Routed     1      2      9            8
Tickysim48   8  8 results/7ed6442_per_node_counters.dat  cyclic        120           10        Routed     5      6      3            4
Tickysim48   8  8 results/7ed6442_per_node_counters.dat  cyclic         53           10        Routed     5      6      3            4
Tickysim48   8  8 results/7ed6442_per_node_counters.dat  cyclic         53            9        Dropped    5      6      3            4
Tickysim48   8  8 results/7ed6442_per_node_counters.dat  cyclic         32            9        Dropped    5      6      3            4
Tickysim48   8  8 results/323309e_per_node_counters.dat  singlesource   16           10        Routed     5      6      3            4
Tickysim48   8  8 results/323309e_per_node_counters.dat  singlesource   32           10        Routed     5      6      3            4
Tickysim48   8  8 results/323309e_per_node_counters.dat  singlesource  120           10        Routed     5      6      3            4
FPGA48       8  8 results/mohsen48_results.csv           cyclic        120            5        Routed     1      2      9            8
FPGA48       8  8 results/mohsen48_results.csv           cyclic         53            5        Routed     1      2      9            8
FPGA48       8  8 results/mohsen48_results.csv           cyclic         53            3        Dropped    1      2      9            8
FPGA48       8  8 results/mohsen48_results.csv           cyclic         32            3        Dropped    1      2      9            8
FPGA48       8  8 results/mohsen48_results.csv           singlesource   16            5        Routed     1      2      9            8
FPGA48       8  8 results/mohsen48_results.csv           singlesource   32            5        Routed     1      2      9            8
FPGA48       8  8 results/mohsen48_results.csv           singlesource  120            5        Routed     1      2      9            8
SpiNNaker48  8  8 results/spinnaker48_results.csv        cyclic        160            6        Routed     1      2      9            8
SpiNNaker48  8  8 results/spinnaker48_results.csv        cyclic         53            6        Routed     1      2      9            8
SpiNNaker48  8  8 results/spinnaker48_results.csv        cyclic         53            5        Dropped    1      2      9            8
SpiNNaker48  8  8 results/spinnaker48_results.csv        cyclic         32            5        Dropped    1      2      9            8
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         27           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         27           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         23           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         23           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         22           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         22           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         21           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         21           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         20           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         20           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         18           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         18           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         16           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  cyclic         16           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement    100           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement    100           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     50           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     50           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     48           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     48           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     46           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     46           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     44           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     44           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     37           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     37           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     23           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  complement     23           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      94           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      94           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      67           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      67           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      64           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      64           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      62           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      62           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      59           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      59           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      41           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      41           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      33           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      33           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      32           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      32           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      31           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      31           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      30           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      30           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      22           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  transpose      22           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado       123           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado       123           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        76           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        76           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        67           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        67           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        64           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        64           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        62           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        62           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        59           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        59           10        Dropped    6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        21           11        Routed     6      7      3            5
Tickysim64   8  8 results/9f8e232_per_node_counters.dat  tornado        21           10        Dropped    6      7      3            5
FPGA64       8  8 results/mohsen64_results.csv           cyclic         27            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         27            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         23            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         23            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         22            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         22            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         21            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         21            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         20            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         20            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         18            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         18            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         16            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           cyclic         16            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement    100            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement    100            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     50            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     50            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     48            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     48            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     46            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     46            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     44            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     44            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     37            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     37            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     23            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           complement     23            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      94            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      94            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      67            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      67            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      64            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      64            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      62            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      62            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      59            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      59            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      41            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      41            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      33            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      33            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      32            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      32            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      31            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      31            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      30            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      30            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      22            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           transpose      22            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado       123            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado       123            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        76            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        76            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        67            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        67            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        64            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        64            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        62            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        62            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        59            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        59            3        Dropped    1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        21            5        Routed     1      2      9            8
FPGA64       8  8 results/mohsen64_results.csv           tornado        21            3        Dropped    1      2      9            8
EOF
)"


# SIMULATOR W  H  FILE                                   PATTERN       INJ_INTERVAL  DATA_COL  DATA_NAME  X_COL  Y_COL  PATTERN_COL  INJ_INTERVAL_COL
EXPERIMENTS="$(cat <<EOF
TickysimSt  12 12 results/per_node_counters2.dat         tornado        53           12        Routed     7      8      3            5
TickysimSf  12 12 results/per_node_counters.dat          tornado        53           12        Routed     7      8      3            5
Tickysim    12 12 results/e74e47a_per_node_counters.dat  tornado        53           10        Routed     5      6      3            4
EOF
)"

echo "$EXPERIMENTS" | while read SIMULATOR W H FILE PATTERN INJ_INTERVAL DATA_COL DATA_NAME X_COL Y_COL PATTERN_COL INJ_INTERVAL_COL; do
	INJ_LOAD="$(python -c "print \"%0.2f\"%(16.0/$INJ_INTERVAL)")"
	
	for DRAW_ROUTES in 0 1; do
		if [ $DRAW_ROUTES -eq 1 ]; then
			OUTFILE="${DRAW_ROUTES_OUT_DIR}/${SIMULATOR}_${DATA_NAME}_${PATTERN}_${INJ_LOAD}.png"
		else
			OUTFILE="${OUT_DIR}/${SIMULATOR}_${DATA_NAME}_${PATTERN}_${INJ_LOAD}.png"
		fi
		
		echo "$OUTFILE"
		
		mkdir -p "$(dirname "$OUTFILE")"
		
		gnuplot -e "SIMULATOR=\"${SIMULATOR}\";" \
		        -e "W=${W};" \
		        -e "H=${H};" \
		        -e "FILE=\"${FILE}\";" \
		        -e "PATTERN=\"${PATTERN}\";" \
		        -e "INJ_INTERVAL=${INJ_INTERVAL};" \
		        -e "DATA_COL=${DATA_COL};" \
		        -e "DATA_NAME=\"${DATA_NAME}\";" \
		        -e "X_COL=${X_COL};" \
		        -e "Y_COL=${Y_COL};" \
		        -e "PATTERN_COL=${PATTERN_COL};" \
		        -e "INJ_INTERVAL_COL=${INJ_INTERVAL_COL};" \
		        -e "UTIL_DIR=\"$(dirname "$0")\";" \
		        -e "DRAW_ROUTES=${DRAW_ROUTES};" \
		        $(dirname "$0")/heatmap_plot.gnuplot \
		  > "$OUTFILE"
	done
done
