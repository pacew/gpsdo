#! /usr/bin/env python3

from gps import *
import datetime
import os

running = True

def getPositionData(gps):
    nx = gpsd.next()
    # For a list of all supported classes and fields refer to:
    # https://gpsd.gitlab.io/gpsd/gpsd_json.html
    print(nx)
    if nx['class'] == 'TPV':
        latitude = getattr(nx,'lat', "Unknown")
        longitude = getattr(nx,'lon', "Unknown")
        print( "Your position: lon = " + str(longitude) + ", lat = " + str(latitude))

gpsd = gps(mode=WATCH_ENABLE|WATCH_NEWSTYLE, host='lab')

for msg in gpsd:
    if msg['class'] == 'SKY':
        now = time.time()
        ts = datetime.datetime.now().strftime('%Y%m%dT%H%M%S')
        print(ts, len(msg['satellites']))

        for sat in msg['satellites']:
            try:
                prn = sat['PRN']
                el = sat['el']
                az = sat['az']
                ss = sat['ss']
            except KeyError as e:
                print(e)
                prn = 0
                el = 0
                az = 0
                ss = 0
                
            if (0 < prn < 128
                and ss > 0
                and el > 0
                and az > 0):
                fname = f'sat{prn:02d}.dat'
                try:
                    mtime = os.path.getmtime(fname)
                except FileNotFoundError:
                    mtime = 0
                with open(fname, 'a') as outf:
                    if now - mtime > 5:
                        outf.write('\n')
                    outf.write(f'{ts} {el} {az} {ss}\n')
            


