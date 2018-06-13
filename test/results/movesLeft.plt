set terminal png size 850,300
set output 'movesLeftDist.png'

red = "#FF0000"; green = "#00FF00"; blue = "#0000FF"; skyblue = "#87CEEB";
set style data histogram
set style histogram cluster 
set style fill solid
#set boxwidth 0.9
set xtics format ""
set grid ytics

set xlabel "moves left"
set ylabel "samples"
set notitle
plot "movesLeftDist.txt" using 2:xtic(1) notitle linecolor rgb skyblue
