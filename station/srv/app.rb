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

def temp2json(t)
	j = {
		time: t[0],
		bus: t[1],
		mission: 0.0
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
	log = `tail -n 10 ../log/bus_temp.csv`.split("\n")
	j = {}
	i = 0
	for data in log do
		bus = data.split(",")
		j.store(i, temp2json(bus))
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
