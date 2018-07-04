set terminal postscript landscape enhanced color
set output 'plotD.ps'
set yzeroaxis
set boxwidth 50
set title "Frequency distribution of subtree sizes"
set xlabel "Size of subtree"
set ylabel "Frequency"
set style fill solid 1.0 noborder
bin_width = 1;
bin_number(x) = floor(x/bin_width)
rounded(x) = bin_width * ( bin_number(x) )
plot [0:*] 'freq' using (rounded($1)):(1) smooth frequency with boxes
set title "Frequency distribution of subtree sizes up to 100 nodes"
set boxwidth 1
plot [0:100] 'freq' using (rounded($1)):(1) smooth frequency with boxes
