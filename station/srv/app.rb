require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

get '/' do
	erb :index
end

def vec2json(v)
	j = {
		time: v[0],
		x: v[1],
		y: v[2],
		z: v[3],
	}
	return j
end

def temp2json(bus, mission)
	j = {
		bus_time: bus[0],
		bus_temp: bus[1],
		mission_time: mission[0],
		mission_temp: mission[1],
	}
	return j
end

get '/data/acc' do
	log = `tail -n 10 ../log/acc.csv`.split("\n")
	j = {}
	i = 0
	for data in log do
		acc = data.split(",")
		j.store(i, vec2json(acc))
		i = i + 1
	end
	json j
end

get '/data/gyro' do
	log = `tail -n 10 ../log/gyro.csv`.split("\n")
	j = {}
	i = 0
	for data in log do
		gyro = data.split(",")
		j.store(i, vec2json(gyro))
		i = i + 1
	end
	json j
end

get '/data/temperature' do
	log_bus = `tail -n 10 ../log/bus_temp.csv`.split("\n")
	log_mission = `tail -n 10 ../log/mission_temp.csv`.split("\n")
	j = {}
	for i in 0..9 do
		bus = []
		mission = []
		if log_bus[i] != nil
			bus = log_bus[i].split(",")
		end
		if log_mission[i] != nil
			mission = log_mission[i].split(",")
		end
		j.store(i, temp2json(bus, mission))
	end
	json j
end

get '/data/pressure' do
	log = `tail -n 10 ../log/pressure.csv`.split("\n")
	j = {}
	i = 0
	for data in log do
		press = data.split(",")
		j.store(i, { time: press[0], press: press[1] })
		i = i + 1
	end
	json j
end

get '/data/gps' do
	time = `tail -n 1 ../log/gps_time.csv`.strip().split(",")
	pos = `tail -n 1 ../log/gps_pos.csv`.strip().split(",")
	j = {
		time: time[0],
		utc: time[1],
		lat: pos[1],
		lng: pos[2]
	}
	json j
end
