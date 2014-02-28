
INJ_RATE = 16.0 / INJ_INTERVAL

# Process the data file into seperate chunks of row-long entries
row_split_cmd = sprintf("awk 'BEGIN{y=-1} {if (y == $%d) {print $0} else {print \"\";print $0}; y = $%d}'", Y_COL, Y_COL)
inj_cmd = sprintf("awk '{if ($%d == \"%d\") print $0}'", INJ_INTERVAL_COL, INJ_INTERVAL)
pattern_cmd = sprintf("awk '{if ($%d == \"%s\") print $0}'", PATTERN_COL, PATTERN)
file_cmd = sprintf("<sort -n -k%d -n -k%d %s | %s | %s | %s", Y_COL, X_COL, FILE, inj_cmd, pattern_cmd, row_split_cmd)

set xrange[0-0.5:12-0.5]
set yrange[0-0.5:12-0.5]
set cbrange[0:]

set title sprintf("%s for %s pattern with %0.2f injected load in %s", DATA_NAME, PATTERN, INJ_RATE, SIMULATOR)
set cblabel DATA_NAME
set ylabel "Y"
set xlabel "X"

set term pngcairo size 800,800

plot file_cmd using (column(X_COL)):(column(Y_COL)):(column(DATA_COL)) with image notitle \
   , "<python util/outline_gen.py 1 1" with lines notitle linecolor rgb "#FFFFFF" lw 2
