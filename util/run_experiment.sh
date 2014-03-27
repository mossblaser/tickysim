#!/bin/bash
#
# Utility script which runs an experiment on a cluster of lab machines
# This script is an absolute mess but very handy non-the-less.
#
# Usage:
#   ./util/run_experiment.sh path/to/config/file.config

# Relative to project root
CONFIG_FILE="$1"

# Count the number of opening brackets in the "Groups" array to yield the number
# of experimental groups. (Note: This is not foolproof. Do not allow fools to
# operate this software.)
NUM_GROUPS="$(( $( grep -A9999 "groups:" "$CONFIG_FILE" \
                   | grep -m1 -B9999 ");" \
                   | sed -re "s/#.*$//" \
                   | tr -c -d "(" \
                   | wc -c \
                 ) - 1 ))"

RESULTS_PREFIX="$(sed -nre 's/.*results_directory: *"([^"]*)" *;.*/\1/p' < "$CONFIG_FILE")"
RESULT_FILES=(global_counters.dat per_node_counters.dat packet_details.dat simulator.dat)
RESULTS_DIR="$(dirname "$RESULTS_PREFIX${RESULT_FILES[0]}" )"

CLUSTER_HEAD_NODE=kilburn.cs.man.ac.uk

PARALLEL_PROFILE=cluster32
THREADS_PER_MACHINE=1

echo =====================
echo Experiment Parameters
echo =====================
echo
echo Config file: $CONFIG_FILE
echo
echo Number of groups: $NUM_GROUPS
echo
echo Results file prefix: $RESULTS_PREFIX
echo Results dir: $RESULTS_DIR
echo Results files: ${RESULT_FILES[@]}
echo
echo Cluster head node: $CLUSTER_HEAD_NODE
echo GNU Parallel Profile: $PARALLEL_PROFILE
echo
echo ==================================
echo Packaging tickysim for the cluster
echo ==================================
make -s dist && \
scp tickysim-0.1.tar.gz $CLUSTER_HEAD_NODE: &&
ssh $CLUSTER_HEAD_NODE "rm -rf tickysim-0.1/; \
                        echo ======================================= && \
                        echo Unpacking/compiling tickysim on cluster && \
                        echo ======================================= && \
                        tar xzfm tickysim-0.1.tar.gz && \
                        cd tickysim-0.1 && \
                        mkdir -p \"$RESULTS_DIR\" && \
                        ./configure -q CFLAGS=-O3 && \
                        make -s && \
                        echo ================== && \
                        echo Running simulation && \
                        echo ================== && \
                        time parallel -J$PARALLEL_PROFILE -j$THREADS_PER_MACHINE \
                          -a <(seq $NUM_GROUPS) \
                          ./src/tickysim_spinnaker \
                            \"$CONFIG_FILE\" \
                            measurements.results_directory=\"${RESULTS_PREFIX}g{1}_\" \
                            experiment.parallel.group={1} \
                        ; \
                        echo ================= && \
                        echo Collating results && \
                        echo ================= && \
                        for RESULT_FILE in ${RESULT_FILES[@]}; do \
                          [ ! -f \"${RESULTS_PREFIX}g1_\$RESULT_FILE\" ] && continue ; \
                          ( head -n1 \"${RESULTS_PREFIX}g1_\$RESULT_FILE\"; \
                            tail -q -n+2 \"${RESULTS_PREFIX}\"g*_\"\$RESULT_FILE\" | sort -n; \
                          ) > \"${RESULTS_PREFIX}\$RESULT_FILE\"; \
                          rm \"${RESULTS_PREFIX}\"g*_\"\$RESULT_FILE\"; \
                        done
                        " && \
echo ====================== && \
echo Downloading results... && \
echo ====================== && \
mkdir -p "$RESULTS_DIR"
scp "${RESULT_FILES[@]/#/$CLUSTER_HEAD_NODE:tickysim-0.1/${RESULTS_PREFIX}}" "$RESULTS_DIR/"

