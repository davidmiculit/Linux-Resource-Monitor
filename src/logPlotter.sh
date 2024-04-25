#!/bin/bash

input_file="resource_log.csv"

output_memory="memory_usage.png"
output_io="io.png"
output_cpu="cpu_usage.png"
output_bandwidthrec="received_bytes.png"
output_bandwidthsent="sent_bytes.png"

# grafic pentru memorie
gnuplot <<EOF
set datafile separator ","
set terminal png size 800,600
set output "$output_memory"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M:%S"
set xlabel "Timestamp"
set ylabel "Memory Usage (kB)"
plot "$input_file" using 1:3 title "Memory Usage" with lines
EOF

#grafic de I/O
gnuplot <<EOF
set datafile separator ","
set terminal png size 800,600
set output "$output_io"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M:%S"
set xlabel "Timestamp"
set ylabel "I/O Usage"
plot "$input_file" using 1:4 title "I/O Usage" with lines
EOF

# grafic pentru CPU usage
gnuplot <<EOF
set datafile separator ","
set terminal png size 800,600
set output "$output_cpu"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M:%S"
set xlabel "Timestamp"
set ylabel "CPU Usage"
plot "$input_file" using 1:2 title "CPU Usage" with lines
EOF

# grafic de received bytes
gnuplot <<EOF
set datafile separator ","
set terminal png size 800,600
set output "$output_bandwidthrec"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M:%S"
set xlabel "Timestamp"
set ylabel "Received Bytes"
plot "$input_file" using 1:5 title "Received Bytes" with lines
EOF

# grafic de sent bytes
gnuplot <<EOF
set datafile separator ","
set terminal png size 800,600
set output "$output_bandwidthsent"
set xdata time
set timefmt "%Y-%m-%d %H:%M:%S"
set format x "%H:%M:%S"
set xlabel "Timestamp"
set ylabel "Sent Bytes"
plot "$input_file" using 1:6 title "Sent Bytes" with lines
EOF

