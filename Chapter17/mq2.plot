set terminal png size 800,600 enhanced font "Helvetica,20"
set output 'mq2.png'
set xdata time
set autoscale
set nokey
set grid lw 1
show grid
set xlabel "\nTime"
set ylabel 'raw'
set format x "%.9f"
set xtics rotate
plot "mq2.log" using ($2/1000000000):($1) with lines
