LOG	:= 4.LOG
LAST:= 25

parse_log: parse_log.cc
	$(CXX) -o $@ $^

log.csv: parse_log
	./parse_log $(LOG) > log.csv

csv: log.csv

all: acc gyro

acc: log.csv
	cat log.csv | grep "acc" > acc.csv
	cat acc.csv | tail -n $(LAST) > acc_last.csv

gyro: log.csv
	cat log.csv | grep "gyro" > gyro.csv
	cat gyro.csv| tail -n $(LAST) > gyro_last.csv

clean:
	rm -f *.csv *.png
	rm -f parse_log
