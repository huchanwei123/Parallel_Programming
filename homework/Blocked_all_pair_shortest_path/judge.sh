#!/bin/bash

for num in {1..17}
do
    echo "  "
    echo "============ $num.in ============"
    if [ $num -ne 5 ]
    then
        ./run.sh testcases/$num.in output/$num.out
        ./judge testcases/$num.ans output/$num.out
    fi
done
echo "All done"

