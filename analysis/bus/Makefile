LOG	:= 4.LOG

parse_log: parse_log.cc
	$(CXX) -o $@ $^

log.csv: parse_log
	./parse_log $(LOG) > log.csv

csv: log.csv

all: acc gyro temp

acc: log.csv
	cat log.csv | grep "acc" > acc.csv

gyro: log.csv
	cat log.csv | grep "gyro" > gyro.csv

temp: log.csv
	cat log.csv | grep "temperature" > temp.csv

clean:
	rm -f *.csv *.png
	rm -f parse_log
