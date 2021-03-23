target extended-remote /dev/ttyBmpGdb
monitor swdp_scan
attach 1
monitor traceswo 2250000 decode
set mem inaccessible-by-default off
