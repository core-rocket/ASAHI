require 'sinatra'
require 'sinatra/reloader'
require 'sinatra/json'

get '/' do
	erb :index
end

get '/data/acc' do
	data = {
		x: 1.0,
		y: 2.0
	}
	json data
end
