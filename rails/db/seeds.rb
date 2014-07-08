# This file should contain all the record creation needed to seed the database with its default values.
# The data can then be loaded with the rake db:seed (or created alongside the db with db:setup).
#
# Examples:
#
#   cities = City.create([{ name: 'Chicago' }, { name: 'Copenhagen' }])
#   Mayor.create(name: 'Emanuel', city: cities.first)
Sequence.create([
  {
    title: "Jägermeister-Party",
    text: "<COLOR g><DOFF><TEXT Jägermeister ><AUTOCENTER><WAIT 1><UP><TEXT Party><AUTOCENTER><WAIT 2><CLOSEMID><SPEED 7><GROUP <COLOR g><BGCOLOR b><LEFT><TEXT 4 cl Jägermeister für 1€><AUTOCENTER><WAIT 1>><CLOSEMID><SPEED 8><OPENRIGHT><COLOR rainbow><TEXT Diverse Jägermeister-Longdrinks><CLOSERIGHT><LINEBREAK>",
    modifiable: false
  },
  {
    title: "Disco Night",
    text: "<SPEED 7><DOWN><TEXT Willkommen><AUTOCENTER><WAIT 3><LINEBREAK><TEXT im BH-Club><AUTOCENTER><WAIT 3><CLOSEMID><WAIT 2><LINEBREAK><GROUP <COLOR g><BGCOLOR b><LEFT><TEXT Heute bei uns:><NOAUTOCENTER><WAIT 1>><LINEBREAK><SPEED 8><FLASH><COLOR rainbow><TEXT Disco Night><AUTOCENTER><WAIT 5><SPEED 9><DSNOW><LINEBREAK><SPEED 8><COLOR r><RIGHT><TEXT 🍷  Desperados Happy Hour  🍷 ><WAIT 1><LINEBREAK><LEFT><TEXT 21:30 - 22:30><WAIT 4><SQUEEZEMID><LINEBREAK><GROUP <COLOR y><BGCOLOR b><DOWN><TEXT Aktuelle><AUTOCENTER><WAIT 2>><GROUP <COLOR y><BGCOLOR b><UP><TEXT Uhrzeit:><AUTOCENTER><WAIT 3>><CLOCK24><CLOCK24><CLOCK24><CLOCK24>",
    modifiable: false
  }
])
