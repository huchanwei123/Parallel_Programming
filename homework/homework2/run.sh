#!/bin/bash
file=$1
thd_per_proc=4
real_lower=-3
real_upper=0.2
imag_lower=-3
imag_upper=0.2
w=245
h=589
output_path="/home/huchanwei123/Desktop/${file}.png"
./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} \
    ${w} ${h} ${output_path}


