# OCC Tools
This directory contains various tools used to develop and debug OCC code.

## occtoolp8
Script can be used to check the status of the OCCs and send commands to the OCCs.
Not all OCC commands are supported for P8.  It can be run from the host or via cronus.
When run from the host, it requires that the opal-prd service to be installed and running:
    apt-get install opal-prd
    service opal-prd status
For TMGT/OCC status: *occtoolp8 -I*
To send POLL command to OCC0: *occtoolp8 -p*
For all available options see script help: *occtoolp8 -h*
