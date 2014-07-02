require 'open3'
require 'twitter'

class SignController < ApplicationController
  def index
    @sequences = Sequence.all
  end

  OpenSSL::SSL::VERIFY_PEER = OpenSSL::SSL::VERIFY_NONE

  def tweets
    
    client = Twitter::REST::Client.new do |config|

      config.consumer_key        = 'Cm6UrYbDOUFsYVlAZP5SvJuEe'

      config.consumer_secret     = 'ZR3ujhZd73KdBWW4FGHbAIsSSFetmjyy5cUgPZ5MBzkh8fe8SZ'

      config.access_token        = '531701148-1VUN5GLJYWBJ4l3mDkH0fdCcPghMG8TBYp4bPIaB'

      config.access_token_secret = 'COuRUdA2WQ5VrMK94wY3vV34HKe0CC88ybviLOGC5mrK6'

    end
 @tweets = client.search("#testtweet", :result_type => "recent").take(3).collect do |tweet|
  "<COLOR y><BGCOLOR b><LEFT>#{tweet.user.screen_name}: #{tweet.text}<WAIT 5>"
end
  puts @tweets.to_s
  render :json => @tweets.to_s
  end

  def commit
    @text = params[:text]
    output, status = Open3.capture2e("./laufschrift_steuerung_bsp", :stdin_data => @text, :binmode => true)
    if status.success?
      render plain: "OK"
    else
      render plain: output, status: :internal_server_error
    end
  end
end
