#!/bin/bash
file=$1
proc=2
thd_per_proc=2
real_lower=-2
real_upper=2
imag_lower=-2
imag_upper=2
w=575
h=575
output_path="./${file}.png"
if [ "$file" == "seq" ]; then
    ./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} \
        ${w} ${h} ${output_path}
else
    #mpirun -np 4 ./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} \
    srun -c${thd_per_proc} -n${proc} ./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} \
		${w} ${h} ${output_path}
	#gdb --args ./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} \
fi

