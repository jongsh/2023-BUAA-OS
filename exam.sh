#!/bin/bash
# 
mkdir mydir

chmod a+rwx mydir

touch myfile
echo "2023" > myfile

mv moveme ./mydir/moveme

cp copyme ./mydir/copied

cat readme

gcc bad.c 2> err.txt

mkdir gen
a=1
if (($# == 0))
then
	while (($a <= 10))
	do
		touch "./gen/$a.txt"
		let a=a+1
	done
else
	while (($a <= $1)) 
        do
                touch "./gen/$a.txt"
		let a=a+1
	done
fi
