#!/bin/bash

NUM_CHIPS=$1
shift

for TABLE_FILE in "$@"; do
	echo "$TABLE_FILE"
	python util/mohsen_table_check.py "$TABLE_FILE" $NUM_CHIPS || exit 1
	./src/tickysim_spinnaker "$TABLE_FILE" || exit 1
done
