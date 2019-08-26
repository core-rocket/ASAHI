set datafile separator ","
set terminal png
set out "motion_last.png"

acc="< cat acc_last.csv"
gyro="< cat gyro_last.csv"

set multiplot layout 2,1
set xlabel "time(sec)"

set ylabel "acc(G)"
plot acc u 2:4 w l title "x", acc u 2:5 w l title "y", acc u 2:6 w l title "z"

set key left bottom
set ylabel "gyro(°/sec)"
plot gyro u 2:4 w l title "x", gyro u 2:5 w l title "y", gyro u 2:6 w l title "z"
