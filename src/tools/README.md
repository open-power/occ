# OCC Tools
This directory contains various tools used to develop and debug OCC code.

## occtoolp9
Script can be used to check the status of the OCCs, send commands to the OCCs, or
read OCC sensors.  It can be run from the host or via cronus.
When run from the host, it requires that the opal-prd service to be installed and running:
    apt-get install opal-prd
    services opal-prd status
For all available options see script help: *occtoolp9 -h*

## ffdcparser
Tool to parse FFDC crash data from the OCC.
Data to be parsed must first be collected: getsram fffbf000 600 -ch 0 -fb ffdc.bin
and then parsed with: ffdcparser ffdc.bin

## check-sensors.sh
This is a build time script that is used to ensure that the list of sensors used by the
OCC are fully populated and added to all relevant sensor lists.
