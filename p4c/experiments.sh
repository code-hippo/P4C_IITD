#!/bin/bash
#	./experiment.sh -b 32 -t 20
#	./experiment.sh -b 32
#
options() {
	cat << EOF
	usage: $0 -option <value>

OPTIONS:
   -h      Show this message
   -b      Set the burst size for the application
   -t      Set the time for application to run
EOF
}

if [ $# -le 1 ]; 
then
	options
	exit 1
fi

if [ $USER != "root" ]; 
then 
	echo "This script must be run with super-user privileges."
	exit 1
fi
TIME=20
while getopts "hb:t:" option;
do
        case $option in
		h) options;;

                b) BURST=$OPTARG;;

                t) TIME=$OPTARG;;
        esac
done
echo -e "Burst size is changed to $BURST"
sed -i -e "s/#define MAX_PKT_BURST.*/#define MAX_PKT_BURST $BURST/" src/hardware_dep/dpdk/includes/dpdk_lib.h
bash run.sh build/l2_switch_test/build/l2_switch_test -- -c 0x4 -n 4 - --log-level 3 -- -p 0x3 --config "\"(0,0,2)(1,0,2)\"" > burst$BURST &
sleep $TIME
kill -9 `pidof l2_switch_test`
echo -e "\n\n\tWait for the graph generation !!!"
mv burst$BURST experiments/
cd experiments
bash graph.sh burst$BURST
cd ..
