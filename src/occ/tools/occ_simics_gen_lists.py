# @file - occ_simics_debug.py
# @brief Create Simics Generic Commands
# 
#   Flag    Def/Fea    Userid    Date        Description
#   ------- ---------- --------  ----------  ----------------------------------
#                      thallet   03/28/2012  Created


from time import sleep
from random import randint
import sys
import re

G_occ_list = []
G_proc_list = []

##############################################################################
###########################################################
#   Functions
###########################################################
##############################################################################

def generate_occlist():
    occsimicslist = cli.quiet_run_command("$occc[0]=0; $b=0; foreach $c in (get-object-list -all occ_simple) {$occc[$b] = $c; $b = $b + 1}")
    num = cli.quiet_run_command("print -d $b")
    for idx in range(int(num[1].rstrip('\n'))):
        occsimicslist = cli.quiet_run_command("echo $occc[" + str(idx) + "]")
        G_occ_list.insert(idx,occsimicslist[1].rstrip('\n'))
    G_occ_list.sort()
    for idx in range(int(num[1].rstrip('\n'))):
        cli.quiet_run_command("$occc[" + str(idx) + "]= " +  G_occ_list[idx] + "")
        print str(idx) + ": " + G_occ_list[idx]

    return

def generate_proclist():
    procsimicslist = cli.quiet_run_command("$proc_chip[0]=0; $f=0; foreach $g in (get-object-list -all cec-chip) {$proc_chip[$f] = $g; $f = $f + 1}")
    num = cli.quiet_run_command("print -d $f")
    for idx in range(int(num[1].rstrip('\n'))):
        procsimicslist = cli.quiet_run_command("echo $proc_chip[" + str(idx) + "]")
        G_proc_list.insert(idx,procsimicslist[1].rstrip('\n'))
    G_proc_list.sort()
    for idx in range(int(num[1].rstrip('\n'))):
        cli.quiet_run_command("$proc_chip[" + str(idx) + "]= " +  G_proc_list[idx] + "")
        print str(idx) + ": " + G_proc_list[idx]

    return



generate_occlist()
generate_proclist()

