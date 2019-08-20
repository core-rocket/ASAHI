#!/bin/bash

# wgrib2 download/Z__C_RJTD_20160810060000_MSM_GPV_Rjp_L-pall_FH00-15_grib2.bin -lon 140 40.24 -match ":*GRD"

# 能代の座標
lat=140.0111
lng=40.2430

# 使う指定気圧面: 1000, 975, 950, 925, 900, 850 (6個)
# 指定気圧面は1000から100で，実測値，予報値(3,6,9,12,15)と並んでいる
# なので，head -n6してしまえば今回使うデータ(実測値)は取れる

# UGRD: 東西風
# VGRD: 南北風

get_value(){
	fname=$1
	data_type=$2
	lines=`wgrib2 $fname -lon $lat $lng -match ":"$data_type | head -n12 | tail -n6`
	for l in $lines; do
		#echo $l
		id=`cut -d':' -f 1 <<< $l`
		d=`cut -d':' -f 3 <<< $l`
		value=`cut -d',' -f 3 <<< $d`
		value=`cut -d'=' -f 2 <<< $value`
		#echo $id
		echo $value
	done
}

extract_data(){
	#fname="download/Z__C_RJTD_20160810060000_MSM_GPV_Rjp_L-pall_FH00-15_grib2.bin"
	fname=$1
	ugrd=(`get_value $fname "UGRD"`)
	vgrd=(`get_value $fname "VGRD"`)

	for ((i=0; i<6; i++)) {
		height=`expr 1000 - 25 \* $i`
		if [ $height = 875 ];then
			height=850
		fi
		echo $height ${ugrd[i]} ${vgrd[i]}
	}
}

extract_all(){
	grib2s=`ls download/*grib2.bin`
	for grib2 in $grib2s; do
		grib2_base=`basename $grib2`
		date=""
		if [[ $grib2_base =~ ^Z__C_RJTD_(.+)_MSM(.+)$ ]];then
			date=${BASH_REMATCH[1]}
		else
			echo error
			exit 1
		fi
		txt=data/$date.txt
		echo -n "writing $txt ... "
		extract_data $grib2 > $txt
		echo "[ok]"
	done
}

#mkdir data
#extract_all
