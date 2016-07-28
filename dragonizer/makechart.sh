#!/bin/sh

# argument 1 is path to input
# set style line 1 ls rgb '#0060ad' lt 1 t

if [ ! -d "$1" ]; then
    echo "specify data directory"
    exit 1
fi

RANGE="[0:5]"

gnuplot << EOF
set terminal png
set output 'time.png'
set title 'Elapsed time according to number of cores'
set xrange $RANGE
plot '$1/pthread.dat' using 1:2 title "pthread" with linespoints, \
     '$1/tbb.dat' using 1:2 title "tbb" with linespoint
EOF

gnuplot << EOF
set terminal png
set output 'speedup.png'
set title 'Speedup according to number of cores'
set xrange $RANGE
plot '$1/pthread.dat' using 1:4 title "pthread" with linespoints, \
     '$1/tbb.dat' using 1:4 title "tbb" with linespoint
EOF

gnuplot << EOF
set terminal png
set output 'efficiency.png'
set title 'Efficiency according to number of cores'
set xrange $RANGE
plot '$1/pthread.dat' using 1:5 title "pthread" with linespoints, \
     '$1/tbb.dat' using 1:5 title "tbb" with linespoint
EOF

