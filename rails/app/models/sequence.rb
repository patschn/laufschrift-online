class Sequence < ActiveRecord::Base
  validates :title, presence: true
end
