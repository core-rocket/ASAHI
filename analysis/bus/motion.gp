set datafile separator ","
set terminal png
set out "motion_last.png"
plot "acc_last.csv" u 2:4 w l title "x", "acc_last.csv" u 2:5 w l title "y", "acc_last.csv" u 2:6 w l title "z"
