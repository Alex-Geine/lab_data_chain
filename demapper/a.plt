set term qt

set xrange [-6:6]
set yrange [-6:6]

set xtics -5,2,5
set ytics -5,2,5

set xzeroaxis
set yzeroaxis

set grid

plot './build/Desktop-Debug/constellation.txt', 'points.txt'

pause mouse close
