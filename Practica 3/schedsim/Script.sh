#! /bin/bash

SCHEDFILE="What file do you want to execute?"
FNREG="The specified file $FILE isn't regular."
FNEXIST="The specified file $FILE doesn't exist."
SCHEDCPUNUM="How many CPUs do you want to simulate? 1-8."
INVALIDCPUNUM="Invalid number of CPUS, it must be between 1-8."

END=0
while [ $END -ne 1 ] ; do
	echo $SCHEDFILE
	read FILE

	if [ -e $FILE ] ; then
		if [ -r $FILE ] ; then
			let END=1
		else
			echo $FNREG
		fi
	else
		echo $FNEXIST
	fi
done

CPUNUM=0
END1=0
while [ $END1 -ne 1 ] ; do
    echo $SCHEDCPUNUM
    read CPUNUM
    if [ $CPUNUM -le 0 ] || [ $CPUNUM -ge 9 ] ; then
	    echo $INVALIDCPUNUM
    else 
        let END1=1
    fi
done


if [ ! -d "resultados" ]
then
	mkdir resultados
fi

for SCHEDNAME in RR SJF FCFS PRIO
do
	for ((CPUS=1; $CPUS<=$CPUNUM; CPUS++))
    do
		./schedsim -i $FILE -n $CPUS -s $SCHEDNAME
		for ((i=1; $i<=$CPUS; i++))
        do
            let "NUM=i-1"
			mv CPU_$NUM.log resultados/$SCHEDNAME-CPU_$NUM.log
		done
		cd ../gantt-gplot
		for ((i=1; $i<=$CPUS; i++))
        do
            let "NUM_1=i-1"
			./generate_gantt_chart ../schedsim/resultados/$SCHEDNAME-CPU_$NUM_1.log
			mv ../schedsim/resultados/$SCHEDNAME-CPU_$NUM_1.eps ../schedsim/resultados/DIAGANTT-$SCHEDNAME-CPU_$NUM_1.eps 
		done
		cd ../schedsim
	done
done
