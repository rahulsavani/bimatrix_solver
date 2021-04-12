set terminal postscript landscape enhanced color
set output 'plotL.ps'
set autoscale
set title "Number of cores vs time"
set xlabel "time (secs)"
set ylabel "cores"
plot 'hist' using 1:2
set title "Size of job list L vs time"
set ylabel "L"
plot 'hist' using 1:3
set title "Requests vs time"
set ylabel "cores"
plot 'hist' using 1:4

