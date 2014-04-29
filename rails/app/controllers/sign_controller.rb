class SignController < ApplicationController
  def index
    @sequence = Sequence.new
    @sequences = Sequence.all
  end

  def commit
    @text = params[:text]
    # TODO: Text übergeben
    #header :not_found
    render plain: "OK"
  end
end
