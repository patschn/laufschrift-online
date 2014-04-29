class SequencesController < ApplicationController
  def show
    begin
      @sequence = Sequence.find(params[:id])
      render json: @sequence
    rescue ActiveRecord::RecordNotFound
      head :not_found
    end
  end

  def create
    @sequence = Sequence.new(sequence_params)
    if @sequence.save
      render json: @sequence, status: :created
    else
      render json: @sequence.errors, status: :unprocessable_entity
    end
  end

  def update
    @sequence = Sequence.find(params[:id])
    if @sequence.update(sequence_params)
      render json: @sequence, status: :accepted
    else
      render json: @sequence.errors, status: :unprocessable_entity
    end
  end

  def destroy
    begin
      Sequence.find(params[:id]).destroy
      render plain: "OK"
    rescue ActiveRecord::RecordNotFound
      head :not_found
    end
  end

  private
  def sequence_params
    params.require(:sequence).permit(:title, :text)
  end
end
