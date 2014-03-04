#!/bin/bash

NUM_CHIPS=$1
PATTERN=$2
shift 2

for TABLE_FILE in "$@"; do
	echo "$TABLE_FILE"
	python util/mohsen_table_check.py "$TABLE_FILE" "$NUM_CHIPS" "$PATTERN" || exit 1
	./src/tickysim_spinnaker "$TABLE_FILE" || exit 1
done
