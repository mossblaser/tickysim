
INJ_RATE = 16.0 / INJ_INTERVAL

# Process the data file into seperate chunks of row-long entries
row_split_cmd = sprintf("python2 %s/pad_heatmap.py %d %d", UTIL_DIR, X_COL, Y_COL)
inj_cmd = sprintf("awk '{if (int($%d) == %d) print $0}'", INJ_INTERVAL_COL, INJ_INTERVAL)
pattern_cmd = sprintf("awk '{if ($%d == \"%s\") print $0}'", PATTERN_COL, PATTERN)
file_cmd = sprintf("<(echo -n '#';head -n1 %s; sort -n -k%d -n -k%d %s | %s | %s) | %s", FILE, Y_COL, X_COL, FILE, inj_cmd, pattern_cmd, row_split_cmd)

print file_cmd

# Draw outlines of boards
outline_cmd = sprintf("<python2 %s/outline_gen.py %d %d", UTIL_DIR, (W+11)/12, (H+11)/12 )

# Draw all routes
if (DRAW_ROUTES) \
	routes_cmd = sprintf("<python2 %s/plot_routes.py %s %d %d", UTIL_DIR, PATTERN, W, H); \
else \
	routes_cmd = sprintf("<python2 %s/plot_routes.py %s %d %d", UTIL_DIR, "disable", W, H);

if (DRAW_ROUTES) \
	set xrange[0-1.5:W-1+1.5]; \
	set yrange[0-1.5:H-1+1.5]; \
else \
	set xrange[0-0.5:W-1+0.5]; \
	set yrange[0-0.5:H-1+0.5];

if (DATA_MAX == 0) \
	set cbrange[0:]; \
else \
	set cbrange[0:DATA_MAX];

set title sprintf("%s for %s pattern with %0.2f injected load in %s", DATA_NAME, PATTERN, INJ_RATE, SIMULATOR)
set cblabel DATA_NAME
set ylabel "Y"
set xlabel "X"

set term pngcairo size 800,800


plot file_cmd using (column(X_COL)):(column(Y_COL)):(column(DATA_COL)) with image notitle \
   , outline_cmd with lines notitle linecolor rgb "#FFFFFF" lw 2 \
   , routes_cmd with lines notitle linecolor rgb "#00FF00" lw 1
