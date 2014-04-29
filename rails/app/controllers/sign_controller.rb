class SignController < ApplicationController
  def index
    @sequence = Sequence.new
    @sequences = Sequence.all
  end

  def commit
    @text = params[:text]
    # TODO: Text übergeben
    render plain: "OK"
  end
end
