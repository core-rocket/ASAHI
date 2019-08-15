require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

get '/' do
	erb :index
end

get '/data/acc' do
	data = {
		x: 0.0,
		y: 1.0,
		z: 2.0
	}
	json data
end
