#! /usr/bin/env python3

import glob
import datetime
import subprocess

now = datetime.datetime.now()
then = now - datetime.timedelta(hours=3.2)

x0 = then.strftime('%Y%m%dT%H%M%S')
x1 = now.strftime('%Y%m%dT%H%M%S')

sat_files = glob.glob("sat*.dat")

for sat in sat_files:
    ssat = f'smooth-{sat}'
    with open(sat) as inf:
        with open(ssat, 'w') as outf:
            smooth = None
            for row in inf:
                row = row.strip()
                if row == '':
                    last = None
                    outf.write('\n')
                else:
                    cols = row.split(' ')
                    cur = float(cols[3])
                    if smooth is None:
                        smooth = cur
                    else:
                        factor = .002
                        smooth = smooth * (1 - factor) + cur * factor

                    cols[3] = str(smooth)
                    outf.write(' '.join(cols))
                    outf.write('\n')


with open ('TMP.gprc', 'w') as outf:
    outf.write('set term png\n')
    outf.write('set output "snr.png"\n')
    outf.write('set xdata time\n')
    outf.write('set timefmt "%Y%m%dT%H%M%S"\n')
    if False:
        outf.write('set link x2\n')
        outf.write('set x2tics format "%m/%d" time\n')
    outf.write('set format x "%H:%M"\n')
    outf.write('set style data lines\n')
    outf.write(f'set xrange ["{x0}":"{x1}"]\n')
    outf.write(f'set yrange [0:50]\n')
    outf.write(f'unset key\n')
    outf.write('set title "GPS signal strength by satellite"\n')
    outf.write(f'set xlabel "Eastern Daylight Time"\n')
    outf.write(f'set ylabel "SNR"\n')

    outf.write('plot')
    comma = ' '
    for sat in sat_files:
        file = f'smooth-{sat}'
        outf.write(f'{comma}"{file}" using 1:4')
        comma = ' ,'
    outf.write('\n')

subprocess.run('gnuplot TMP.gprc', shell=True)
