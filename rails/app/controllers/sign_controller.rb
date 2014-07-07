require 'open3'
require 'twitter'

class SignController < ApplicationController
  def index
    @sequences = Sequence.all
  end

  OpenSSL::SSL::VERIFY_PEER = OpenSSL::SSL::VERIFY_NONE

  def tweets
    client = Twitter::REST::Client.new do |config|
      config.consumer_key        = '8La81KYPmMBQ8SBsS3VDFiD38'
      config.consumer_secret     = 'QhlvKjNWSU0FdDR1UMMzwwVbQ2ebDLI1dmf47a4sVHlY0GgBIV'
      config.access_token        = '2603579250-qfFJXGyU1gzTIlA1y1ODAJWa1pdJVQTbf8SSgAC'
      config.access_token_secret = 'xYiC6327ZVaPGayi1uoL2DUhelRej14wKAVBXxyNM81nZ'
    end
    @tweets = client.search("#bhclub", :result_type => "recent").take(3).collect do |tweet|
      [ "#{tweet.id}", "#{tweet.user.screen_name}: #{tweet.text}" ]
    end
    #puts @tweets.to_s
    render :json => @tweets.to_s
  end

  def commit
    @text = params[:text]
    output, status = Open3.capture2e("./laufschrift_steuerung", :stdin_data => @text, :binmode => true)
    if status.success?
      render plain: "OK"
    else
      render plain: output, status: :internal_server_error
    end
  end
end
