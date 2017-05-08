load tclgif
set w 320
set h 240

tclgif::new matze.gif $w $h 8 2

# black red green white
tclgif::colput 0   0   0   0
tclgif::colput 1 255   0   0
tclgif::colput 2   0 255   0
tclgif::colput 3 255 255 255

# default color
tclgif::coldef 3

set i [image create photo -width $w -height $h]

# 1. red
$i put red -to 0 0 $w $h
tclgif::add $i 100

# 2. default
$i put blue -to 0 0 $w $h
tclgif::add $i 100

# 3. green
$i put green -to 0 0 $w $h
tclgif::add $i 100

# 4. black
$i put black -to 0 0 $w $h
tclgif::add $i 100

# 5. default
$i put yellow -to 0 0 $w $h
tclgif::add $i 100

# 6. white
$i put white -to 0 0 $w $h
tclgif::add $i 100

tclgif::close

puts "ready ..."