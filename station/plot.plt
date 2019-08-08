# plot frame

set multiplot layout 3,1

acc_data="< tail -n 500 log/acc.csv"

plot acc_data u 1:2 w l title "acc.x"
plot acc_data u 1:3 w l title "acc.y"
plot acc_data u 1:4 w l title "acc.z"
