require 'open3'	

class SignController < ApplicationController
  def index
    @sequences = Sequence.all
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
