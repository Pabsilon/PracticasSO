#! /bin/bash

SCHEDFILE="What file do you want to execute?"
FNREG="The specified file $FILE isn't regular."
FNEXIST="The specified file $FILE doesn't exist."
SCHEDCPUNUM="How many CPUs do you want to simulate? 1-8."

FOUND=0
while [ $FOUND -ne 1 ] ; do
	echo $SCHEDFILE
	read FILE

	if [ -e $FILE ] ; then
		if [ -r $FILE ] ; then
			let FOUND=1
		else
			echo $FNREG
		fi
	else
		echo $FNEXIST
	fi
done
CPUNUM=0
echo $SCHEDCPUNUM
read CPUNUM
	if [ $CPUNUM -le 0 ] ; then
		let CPUNUM = 1
	fi
	if [ $CPUNUM -ge 9 ] ; then
		let CPUNUM = 8
	fi
echo $CPUNUM




mkdir resultados

for SCHEDNAME in 'seq RR SJF FCFS PRIO' do
	for (CPUS=1; $CPUS<=$CPUNUM; CPUS++;) do
		./schedsim -i $FILE -n $CPUS -s $SCHEDNAME
			for (i=1; $i<=$CPUNUM i++) do
				mv CPU_$i.log resultados/$SCHEDNAME-CPU_$i.log
			done
		cd ../gantt-plot
		for (i=1; $i<=$CPUNUM; i++) do
			./generate_gantt_chart ../schedsim/resultados/$SCHEDNAME-CPU_$i.log
			mv CPU_$i.eps ../schedsim/resultados/DIAGANTT-$SCHEDNAME-CPU_$i.eps 
		done
		cd ../schedsim
	done
done