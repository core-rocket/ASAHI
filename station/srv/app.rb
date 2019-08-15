require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

get '/' do
	erb :index
end

get '/data/acc' do
	log = `tail -n 1 ../log/acc.csv`
	acc = log.split(",")
	j = {
		time: acc[0],
		x: acc[1],
		y: acc[2],
		z: acc[3]
	}
	json j
end
