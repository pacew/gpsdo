target extended-remote /dev/ttyBmpGdb
monitor swdp_scan
attach 1
monitor traceswo
set mem inaccessible-by-default off
