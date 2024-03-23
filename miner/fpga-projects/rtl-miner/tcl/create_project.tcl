###############################################################################
# Set project paths and create project
###############################################################################

set tcl_path [file dirname [file normalize [info script]]]
set top_path [file dirname $tcl_path]
set prj_path "$top_path/prj"

puts "The project path is: $prj_path"

set part "xc7z007sclg400-1"
set project_name "miner"

create_project $project_name $prj_path -part $part -force

# TODO: how to use board files via TCL
#set obj [current_project]
#set_property -name "board_part" -value "digilentinc.com:cora-z7-07s:part0:1.1" -objects $obj

###############################################################################
# This script will add all the rtl sources for the project
###############################################################################

set src_path "$top_path/src"
add_files -fileset sources_1 $src_path 

set sim_path "$top_path/sim"
add_files -fileset sim_1 $sim_path 

###############################################################################
# Here we add the block design to the project
###############################################################################

set bd_path "$top_path/bd"
source $bd_path/ProcessingSystem.tcl

###############################################################################
# Build project
###############################################################################

# Compile
update_compile_order -fileset sources_1

# Shortcut to bit stream generation
launch_runs impl_1 -to_step write_bitstream -jobs 16
wait_on_run impl_1

# Export hardware description
write_hw_platform -fixed -force -file $prj_path/top.xsa
