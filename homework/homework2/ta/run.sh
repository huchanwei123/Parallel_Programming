#!/bin/bash
file=$1
thd_per_proc=4
real_lower=-2
real_upper=2
imag_lower=-2
imag_upper=2
w=575
h=575
output_path="/home/huchanwei123/Desktop/Parallel_Programming/homework/homework2/${file}.png"
./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} \
    ${w} ${h} ${output_path}


