class AddModifiableToSequences < ActiveRecord::Migration
  def change
    add_column :sequences, :modifiable, :boolean, default: true, after: :text
  end
end
