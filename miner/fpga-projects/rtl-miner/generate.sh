SCRIPT=$(readlink -f $0)
SCRIPT_PATH=`dirname $SCRIPT` 
PROJECT_PATH=${SCRIPT_PATH}/prj
echo -e "The project path is: ${PROJECT_PATH}"
cd ${PROJECT_PATH}
vivado -mode batch -source ../tcl/create_project.tcl