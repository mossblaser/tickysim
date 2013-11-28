#!/bin/bash

# Utility script which runs an experiment on a cluster of lab machines
# This script is an absolute mess but very handy non-the-less.

# Relative to packet root
CONFIG_FILE="configs/mohsen.config"
NUM_GROUPS=22
NUM_SAMPLES=1

RESULTS_DIR="results"
RESULT_FILES="global_counters.dat per_node_counters.dat packet_details.dat simulator.dat"

CLUSTER_HEAD_NODE=kilburn.cs.man.ac.uk

PARALLEL_PROFILE=cluster64

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
                        ./configure -q && \
                        make -s && \
                        echo ================== && \
                        echo Running simulation && \
                        echo ================== && \
                        time parallel -J$PARALLEL_PROFILE -j1 \
                          -a <(seq $NUM_GROUPS) \
                          -a <(seq $NUM_SAMPLES) \
                          ./src/tickysim_spinnaker \
                            \"$CONFIG_FILE\" \
                            measurements.results_directory=\"$RESULTS_DIR/g{1}_s{2}_\" \
                            experiment.parallel.group={1} \
                            experiment.parallel.sample={2} \
                        && \
                        echo ================= && \
                        echo Collating results && \
                        echo ================= && \
                        cd \"$RESULTS_DIR\" && \
                        for RESULT_FILE in $RESULT_FILES; do \
                          [ ! -f g1_s1_\"\$RESULT_FILE\" ] && continue ; \
                          ( head -n1 g1_s1_\"\$RESULT_FILE\"; \
                            tail -q -n+2 g*_s*_\"\$RESULT_FILE\" | sort -n; \
                          ) > \"\$RESULT_FILE\"; \
                          rm g*_s*_\"\$RESULT_FILE\"; \
                        done
                        " && \
echo ====================== && \
echo Downloading results... && \
echo ====================== && \
scp "$CLUSTER_HEAD_NODE:tickysim-0.1/$RESULTS_DIR/*" $RESULTS_DIR/

