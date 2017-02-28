#!/bin/bash
file=$1
cp $1 avg_$1
sed -i "/[a-zA-Z]/d" $file
sed -i '/^$/d' $file
head -n 500050 $file > temp
tail -n +50 temp > $file
rm temp
awk '{ total += $1; count++; if(count == "1000") {print NR,total/count; total = 0; count = 0;}}' $file > smac_avg_every_1000
awk '{ total += $2; count++; if(count == "1000") {print NR,total/count; total = 0; count = 0;}}' $file > dmac_avg_every_1000
echo 'set autoscale' > plot.ps
echo 'unset log' >> plot.ps
echo 'unset label' >> plot.ps
echo 'set tics font ", 12"' >> plot.ps
echo 'set yrange [0:]' >> plot.ps
echo 'set ytics 10' >> plot.ps
echo 'set xtics 10000 rotate by 90 right' >> plot.ps
echo 'set grid back' >> plot.ps
echo 'set title "smac lookup average/1000 packets"' >> plot.ps
echo 'set xlabel "Total Number of Packets"' >> plot.ps
echo 'set ylabel "Avg CPU Cycles for lookup"' >> plot.ps
#echo 'set terminal postscript portrait enhanced mono dashed lw 1 "Helvetica" 14' >> plot.ps
echo 'set terminal png size 1024,768 enhanced font "Helvetica,12"' >> plot.ps
echo 'set output "smac_lookup_time_for_'$file'.png"' >> plot.ps
echo 'plot "smac_avg_every_1000" using 1:2 title "" with line linetype 1 linecolor 1 linewidth 2' >> plot.ps
gnuplot plot.ps
sed -i "s/smac/dmac/g" plot.ps
gnuplot plot.ps
rm plot.ps
rm smac_avg_every_1000
rm dmac_avg_every_1000
rm $file
