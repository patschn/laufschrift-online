class CreateSequences < ActiveRecord::Migration
  def change
    create_table :sequences do |t|
      t.string :title
      t.text :text

      t.timestamps
    end
  end
end
