#!/bin/bash
version=$1
rm -r output/*
rm ${version}
make judge
make ${version}
for num in {3..3}
do
    echo "  "
    echo "============ $num.in ============"
    if [ $num -ne 5 ] && [ $num -ne 5 ] 
    then
        ./run.sh ${version} /home/pp18/shared/hw4/testcases/$num.in output/$num.out
        ./judge /home/pp18/shared/hw4/testcases/$num.ans output/$num.out
    fi
done
echo "All done"

