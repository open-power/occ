# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/tools/simics/p10_occ_setup.py $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2019,2020
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

# Syntax Check: python -m py_compile p10_occ_setup.py

from __future__ import print_function

from cli import *
import random
import time
import sys
import os
from curses.ascii import isprint


script_directory = os.path.dirname(os.path.realpath(__file__))
#tools_directory = /gsa/rchgsa/projects/p/power_thermal/bin"
tools_directory = "/gsa/ausgsa/projects/o/occfw/simics_p10/bin"


CONST_VERSION = 'P10.2.0'
LastCmd_SeqNum = 00
L_pgpe_enabled = 0

# Simics is not actively running (issue manual run commands between operations)
G_run_time = 1
# Set bypass to 1 to bypass the bootloader and start the 405 code directly
G_bypass_bootloader = 0

# P10
proc = "dcm0.chip0"
if 1 == 1:
    G_occsram_trace_length = "0x2400"
    G_occsram_err_trace  = "0xFFFF6400" # New SRAM locations 10/10/2019
    G_occsram_inf_trace  = "0xFFFF8800"
    G_occsram_imp_trace  = "0xFFFFAC00"
    G_occsram_cmd_buffer = "0xFFFFD000"
    G_occsram_rsp_buffer = "0xFFFFE000"

G_occ_rsp_buffer_size = 4096


def helper(callStr):
    print("\n\n##################################################################")
    print("OCCw Verion : " + CONST_VERSION )
    print("Support : Chris Cain / Sheldon Bailey")
    print("Location: " + script_directory)
    print("##################################################################")
    print('OCCwrite -C 0x<CMD> -D "<data>" ')
    print('OCCw     -C 0x<CMD> -D "<data>" ')
    print("            0x<CMD>                  must be in hex and have the 0x in front of it.")
    print('                       "<data>"      must be in hex without the 0x and have quotes around it.')
    print("")
    print("OCCread  -C 0x<CMD> ")
    print("OCCr     -C 0x<CMD> ")
    print("            0x<CMD>                  must be in hex and have the 0x in front of it.")
    print("")
    print("Example:")
    print('    OCCw  -C 0x00 -D "20"             : OCC_CMD_POLL write')
    print("    OCCr  -C 0x00                     : OCC_CMD_POLL read")
    print("")
    print("Other Commands ")
    print('    OCCtool  -C 0x01 -D "<data>"      : OCC_CMD_QUERY_FIRMWARE_LEVEL')
    print(" ")
    print('    OCCtool  -C 0x10 -D "<data>"      : OCC_CMD_GET_ERROR_LOG')
    print('    OCCtool  -C 0x11 -D "<data>"      : OCC_CMD_CONTINUE_ERROR_LOG')
    print('    OCCtool  -C 0x12 -D "<data>"      : OCC_CMD_CLEAR_ERROR_LOG')
    print(" ")
    print('    OCCtool  -C 0x20 -D "<data>"      : OCC_CMD_SET_MODE_AND_STATE')
    print('    OCCtool  -C 0x21 -D "<data>"      : OCC_CMD_SETUP_CONFIGURATION_DATA')
    print('    OCCtool  -C 0x23 -D "<data>"      : OCC_CMD_SET_THERMAL_THROTTLE_MODE')
    print('    OCCtool  -C 0x24 -D "<data>"      : OCC_CMD_SET_PEX_REGISTER')
    print('    OCCtool  -C 0x25 -D "<data>"      : OCC_CMD_RESET_PREP')
    print('    OCCtool  -C 0x26 -D "<data>"      : OCC_CMD_SNAPSHOT_SYNC')
    print(" ")
    print('    OCCtool  -C 0x30 -D "<data>"      : OCC_CMD_PEX_PASS_THROUGH')
    print('    OCCtool  -C 0x31 -D "<data>"      : OCC_CMD_READ_STATUS_REGISTER')
    print('    OCCtool  -C 0x32 -D "<data>"      : OCC_CMD_GET_THROTTLE_VALUES')
    print('    OCCtool  -C 0x33 -D "<data>"      : OCC_CMD_GET_CPU_TEMPERATURES')
    print('    OCCtool  -C 0x34 -D "<data>"      : OCC_CMD_GET_COOLING_REQUEST')
    print('    OCCtool  -C 0x35 -D "<data>"      : OCC_CMD_GET_OCC_SNAPSHOT')
    print(" ")
    print('    OCCtool  -C 0x40 -D "<data>"      : OCC_CMD_DEBUG_PASS_THROUGH')
    print('    OCCtool  -C 0x41 -D "<data>"      : OCC_CMD_AME_PASS_THROUGH')
    print('    OCCtool  -C 0x42 -D "<data>"      : OCC_CMD_GET_FIELD_DEBUG_DATA')
    print(" ")
    print('    OCCtool  -C 0x50 -D "<data>"      : OCC_CMD_VRM_INTERFACE_TEST')
    print('    OCCtool  -C 0x51 -D "<data>"      : OCC_CMD_PROCESSOR_INTERFACE_TEST')
    print('    OCCtool  -C 0x52 -D "<data>"      : OCC_CMD_SET_PROCESSOR_CLOCK_FREQ')
    print('    OCCtool  -C 0x53 -D "<data>"      : OCC_CMD_MFG_TEST')
    print('    OCCtool  -C 0x54 -D "<data>"      : OCC_CMD_GET_FOM_DATA')
    print(" ")
    print('    OCCtool  -C 0x60 -D "<data>"      : OCC_CMD_TUNABLE_PARAMETERS')
    print(" ")
    print("##################################################################")
    print("Reason for HELP (" + callStr + ")\n\n")


# Given a hex string array, dump the data in hex:
#   Input: intArray=['0102','0304','00AA,0xBBAA, ...]
#   Output:
#     0x0000:  01020304 00AABBAA 01000000 0500004E  "...............N"
#     0x0010:  48021000 0000AAAA AA020000 00050000  "H........ ......"
#   ...
def hexDumpString(stringArray):
    offset=0
    for hword in stringArray:
        if ((offset % 16) == 0):
            print("{0:#0{1}x}".format(offset,6)+":  ", end="")
        if ((offset % 4) == 0):
            print(" ", end="")
        print(hword.upper(), end="")
        offset=offset+2
        if ((offset % 16) == 0):
            print("")
    if ((offset % 16) != 0):
        print("")

# Given a hex string array, dump the data in hex:
#   Input: intArray=['01','02','03','04','00','AA','BB','AA', ...]
#   Output:
#     0x0000:  01020304 00AABBAA 01000000 0500004E  "...............N"
#     0x0010:  48021000 0000AAAA AA020000 00050000  "H........ ......"
#   ...
def hexDumpByteString(stringArray):
    offset=0
    for hword in stringArray:
        if ((offset % 16) == 0):
            print("{0:#0{1}x}".format(offset,6)+":  ", end="")
        if ((offset % 4) == 0):
            print(" ", end="")
        print(hword.upper(), end="")
        offset=offset+1
        if ((offset % 16) == 0):
            print("")
    if ((offset % 16) != 0):
        print("")

# Given a hex string array, dump the data in hex:
#   Input: intArray=["0102030400AABBAA..."]
#   Output:
#     0x0000:  01020304 00AABBAA 01000000 0500004E  "...............N"
#     0x0010:  48021000 0000AAAA AA020000 00050000  "H........ ......"
#   ...
def hexDumpSingleString(stringArray):
    offset=0
    for index in xrange(0, len(stringArray)/2):
        if ((offset % 16) == 0):
            print("{0:#0{1}x}".format(offset,6)+":  ", end="")
        if ((offset % 4) == 0):
            print(" ", end="")
        print(stringArray[(offset*2)].upper()+stringArray[(offset*2)+1].upper(), end="")
        offset=offset+1
        if ((offset % 16) == 0):
            print("")
    if ((offset % 16) != 0):
        print("")

def hexDumpPanic(stringArray):
    hex_filename = "/tmp/p10-OCC_panic.txt"
    hex_file = open(hex_filename, "w")
    offset=0
    for hword in stringArray:
        if ((offset % 16) == 0):
            print("{0:#0{1}x}".format(offset,6)+":  ", end="")
            hex_file.write("{0:#0{1}x}".format(offset,6)+":  ")
        if ((offset % 4) == 0):
            print(" ", end="")
            hex_file.write(" ")
        print(hword, end="")
        hex_file.write(hword)
        offset=offset+2
        if ((offset % 16) == 0):
            print("")
            hex_file.write("\n")
    if ((offset % 16) != 0):
        print("")
        hex_file.write("\n")
    hex_file.close()
    print("Panic data written to "+hex_filename)
    command = "!"+tools_directory+"/asm2bin "+hex_filename
    print("==> Converting Panic to binary:" + command)
    run_command(command)
    command = "!"+tools_directory+"/ffdcparser "+hex_filename+".bin"
    print("==> Parsing Panic: " + command)
    run_command(command)

# Given a integer array, dump the data in hex:
#   Input: intArray=[0x01,0x02,0x03,0x04,0x00,0xAA,0xBB,...]
#   Output:
#     0x0000:  01020304 00AABBAA 01000000 0500004E  "...............N"
#     0x0010:  48021000 0000AAAA AA020000 00050000  "H........ ......"
#   ...
def hexDumpInt(intArray):
    # Display data buffer in hex dump
    asciiData=""
    for i in xrange(0, len(intArray)):
        if (i % 16 == 0):
            if (i > 0):
                print("  \""+asciiData+"\"")
                asciiData=""
            print('0x%04X' % i, end=": ")
        if (i % 4 == 0):
            print(" ", end="")
        print('%02X' % intArray[i], end="")
        asciiChar=chr(intArray[i])
        if (isprint(asciiChar)):
            asciiData+=str(chr(intArray[i]))
        else:
            asciiData+="."
    print("")

# Given a byte and arry of strings, return string of active bits
#  Input: value = 0xAB, or value = "AB", flags=["bit0","","bit2","bit3",...]
#  Returns: "bit0,bit2,bit3"
flags=["Bit 0", "Bit 1", "", "Bit 3"]
def parseBitField(value, flags):
    flagLength=len(flags)
    if type(value) == str:
        fieldValue=int(value,16)
    else:
        fieldValue=value

    result=""
    sep=","
    if (fieldValue != 0):
        if (flagLength > 0) and (fieldValue & 0x80) : result+=flags[0]+sep
        if (flagLength > 1) and (fieldValue & 0x40) : result+=flags[1]+sep
        if (flagLength > 2) and (fieldValue & 0x20) : result+=flags[2]+sep
        if (flagLength > 3) and (fieldValue & 0x10) : result+=flags[3]+sep
        if (flagLength > 4) and (fieldValue & 0x08) : result+=flags[4]+sep
        if (flagLength > 5) and (fieldValue & 0x04) : result+=flags[5]+sep
        if (flagLength > 6) and (fieldValue & 0x02) : result+=flags[6]+sep
        if (flagLength > 7) and (fieldValue & 0x01) : result+=flags[7]+sep
    # Strip off last separator
    if (result != ""):
        result = result[:-1]
    return result


def getRcString(rc):
    if   (rc == 0x00): string="Success"
    elif (rc == 0x11): string="Invalid Command"
    elif (rc == 0x12): string="Invalid Command Length"
    elif (rc == 0x13): string="Invalid Data Field"
    elif (rc == 0x14): string="Checksum Failure"
    elif (rc == 0x15): string="Internal Error"
    elif (rc == 0x16): string="Present State Prohibits"
    elif (rc == 0xE0): string="OCC Exception: Panic"
    elif (rc == 0xE1): string="OCC Exception: Initialization Checkpoint"
    elif (rc == 0xE2): string="OCC Exception: Watchdog Timeout"
    elif (rc == 0xE3): string="OCC Exception: OCB Timeout"
    elif (rc == 0xE4): string="OCC Exception: HW Error (OCCLFIR bit)"
    elif (rc == 0xE5): string="OCC Exception: Initialization Error"
    elif (rc == 0xFF): string="Command in Progress"
    else:              string="UNKNOWN"
    return string


def getCmdString(cmd):
    if   (cmd == 0x00): string="POLL"
    elif (cmd == 0x12): string="CLEAR_ERROR_LOG"
    elif (cmd == 0x20): string="SET_MODE_AND_STATE"
    elif (cmd == 0x21): string="SET_CONFIG_DATA"
    elif (cmd == 0x22): string="SET_POWER_CAP"
    elif (cmd == 0x25): string="RESET_PREP"
    elif (cmd == 0x26): string="SNAPSHOT_SYNC"
    elif (cmd == 0x35): string="GET_SNAPSHOT_BUFFER"
    elif (cmd == 0x40): string="DEBUG_PASSTHRU"
    elif (cmd == 0x41): string="AME_PASSTHRU"
    elif (cmd == 0x42): string="GET_FIELD_DEBUG_DATA"
    elif (cmd == 0x53): string="MFG_TEST"
    else:               string="UNKNOWN"
    return string


def getConfigString(config):
    if   (config == 0x00): string=""
    elif (config == 0x02): string="Frequency Points"
    elif (config == 0x03): string="OCC Role"
    elif (config == 0x04): string="APSS Configuration"
    elif (config == 0x05): string="Memory Configuration"
    elif (config == 0x07): string="Power Cap Values"
    elif (config == 0x0F): string="System Configuration"
    elif (config == 0x11): string="Idle Power Saver Settings"
    elif (config == 0x12): string="Memory Throttle Data"
    elif (config == 0x13): string="Thermal Control Thresholds"
    elif (config == 0x14): string="AVS Bus Configuration"
    elif (config == 0x15): string="GPU Configuration"
    elif (config == 0x21): string="VRM Fault"
    else:                  string="UNKNOWN"
    return string

def ReadSRAM(command, FLG_VRB):

    ReturnData = []
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    # Errors are reported using CliError exception.
    # returns a tuple
    #   return value as ﬁrst entry,
    #   and the text output as the second.
    # output modes:
    #   output_modes.regular            formatted text with Simics-internal markup
    #   output_modes.formatted_text     formatted text without markup
    #   output_modes.unformatted_text   unformatted text without markup
    #

    mycollapsedstring = ' '.join(ReturnData[1].split())
    tmplist = mycollapsedstring.split(" ")


    import string
    hex_digits = set(string.hexdigits)
    ReturnList = []
    N = 0
    for N in xrange(0, len(tmplist)):
        if(len(tmplist[N]) == 4) and ( all(c in hex_digits for c in tmplist[N]) == True ):
            ReturnList.append( tmplist[N] )
        #     print("KEEP   : " + str(N) + " : " + tmplist[N] )
        # else:
        #     print("DELETE : " + str(N) + " : " + tmplist[N] )

    return (ReturnData[0], ReturnList)


def ReadPrint_CMD(FLG_VRB, data_length):
    RC = 0
    offset = 0

    if data_length <= 0:
        data_length = 0x100
    RC, mylist = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_cmd_buffer+" "+str(data_length), FLG_VRB)

    print("OCC Command Buffer:")
    hexDumpString(mylist)
    print("ReturnCode : " + str(RC))

    return RC

def ReadPrint_POLL(FLG_VRB, SeqNum):
    RC = 0
    CheckSum = 0
    Successful = 0

    sramRC, mylist = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_rsp_buffer+" "+str(G_occ_rsp_buffer_size), FLG_VRB)

    # loop through the data and calculate checksum.
    # sheldon9999 make checksum calculations more of algorithem than this.
    CheckSum += int( mylist[0][:2], 16 )
    CheckSum += int( mylist[0][-2:], 16 )
    CheckSum += int( mylist[1][:2], 16 )
    CheckSum += int( mylist[1][-2:], 16 )
    CheckSum += int( mylist[2][:2], 16 )
    CheckSum += int( mylist[2][-2:], 16 )
    CheckSum += int( mylist[3][:2], 16 )
    CheckSum += int( mylist[3][-2:], 16 )
    CheckSum += int( mylist[4][:2], 16 )
    CheckSum += int( mylist[4][-2:], 16 )
    CheckSum += int( mylist[5][:2], 16 )
    CheckSum += int( mylist[5][-2:], 16 )
    CheckSum += int( mylist[6][:2], 16 )
    CheckSum += int( mylist[6][-2:], 16 )
    CheckSum += int( mylist[7][:2], 16 )
    CheckSum += int( mylist[7][-2:], 16 )
    CheckSum += int( mylist[8][:2], 16 )
    CheckSum += int( mylist[8][-2:], 16 )
    CheckSum += int( mylist[9][:2], 16 )
    CheckSum += int( mylist[9][-2:], 16 )
    CheckSum += int( mylist[10][:2], 16 )

    ReturnCheckSum = mylist[10][-2:] + mylist[11][:2]

    #CheckSum += 1       #sheldonTEST

    if(FLG_VRB):
        print("Calculated Checkstop : " + str( hex( CheckSum ) ) + "\n\n")

    if( sramRC == None) and ( CheckSum == int(ReturnCheckSum, 16) ):
        print("Good Call Return from OCCread, and Good CheckSum " )
        print("======================================================")
        print("\tSeq Num                : " + mylist[0][:2],end="")
        #SeqNum = '10'                                 #sheldonTEST
        if(mylist[0][:2] == SeqNum):
            print("\tValidation - Matched Last CMD")
            Successful += 1
        elif(SeqNum == '00'):
            print("\tRead only no Validation.")
            Successful += 1
        else:
            print("\tValidation FAILED - Last CMD Sequence Number : " + SeqNum)
        cmdString=getCmdString(int(mylist[0][-2:],16))
        print("\tCommand                : " + mylist[0][-2:] + " : " + cmdString)
        rcString=getRcString(int(mylist[1][:2],16))
        print("\tReturn Status          : " + mylist[1][:2] + " : " + rcString)
        print("\tData Length            : " + mylist[1][-2:] + mylist[2][:2] )
        #######################################################################################################
        statusBits=["Master","Collect Fir Data","","OCC owns PMCR","Simics","","Observation Ready","Active Ready"]
        values = parseBitField(mylist[2][-2:], statusBits)
        print("\tStatus (1)             : " + "0x%08X" % int(mylist[2][-2:],16) + " : " + values)
        print("\tStatus (1)             : " + mylist[2][-2:] + " : " + "{0:08b}".format( int(mylist[2][-2:], 16))  )
        if( int(mylist[2][-2:], 16) & 0x80  ):
            print("\t\t\t\t\tBit(7) : Master OCC")
        else:
            print("\t\t\t\t\tBit(7) : Slave OCC")
        if( int(mylist[2][-2:], 16) & 0x40  ):
            print("\t\t\t\t\tBit(6) : Collect FIR Data")
        else:
            print("\t\t\t\t\tBit(6) : Not collecing FIR Data")
        print("\t\t\t\t\tBit(5) : Reserved")
        if( int(mylist[2][-2:], 16) & 0x10  ):
            print("\t\t\t\t\tBit(4) : Status Change -> pwr/thermal change needs to be reported.")
        else:
            print("\t\t\t\t\tBit(4) : State Change - N/A")
        if( int(mylist[2][-2:], 16) & 0x08  ):
            print("\t\t\t\t\tBit(3) : ATTNs - ENABLED")
        else:
            print("\t\t\t\t\tBit(3) : ATTNs - DISABLED")
        print("\t\t\t\t\tBit(2) : Reserved")
        if( int(mylist[2][-2:], 16) & 0x02  ):
            print("\t\t\t\t\tBit(1) : OCC Observation Ready - ENABLED")
        else:
            print("\t\t\t\t\tBit(1) : OCC Observation Ready - DISABLED")
        if( int(mylist[2][-2:], 16) & 0x01  ):
            print("\t\t\t\t\tBit(0) : OCC Active READY - ENABLED")
        else:
            print("\t\t\t\t\tBit(0) : OCC Active READY - DISABLED")
        #######################################################################################################
        statusBits=["DVFS due to Proc OT","DVFS due to Power","Memory Trottle due to OT","Quick Power Drop","DVFS due to Vdd OT"]
        values = parseBitField(mylist[2][-2:], statusBits)
        print("\tExt Status (2)         : " + "0x%08X" % int(mylist[3][:2],16) + " : " + values)
        print("\tExt Status (2)         : " + mylist[3][:2] + " : " + "{0:08b}".format( int(mylist[3][:2], 16))   )
        if( int(mylist[3][:2], 16) & 0x80  ):
            print("\t\t\t\t\tBit(7) : DVFS due to OT - Freq. below nominal due to over temp (proc or VRM)")
        else:
            print("\t\t\t\t\tBit(7) : DVFS due to OT - DISABLED")
        if( int(mylist[3][:2], 16) & 0x40  ):
            print("\t\t\t\t\tBit(6) : DVFS due to Power - Freq. below nominal due to reaching current power cap limit")
        else:
            print("\t\t\t\t\tBit(6) : DVFS due to Power - DISABLED")
        if( int(mylist[3][:2], 16) & 0x20  ):
            print("\t\t\t\t\tBit(5) : Memory throttle due to OT - Memory throttled due to over temp")
        else:
            print("\t\t\t\t\tBit(5) : Memory throttle due to OT - DISABLED")
        if( int(mylist[3][:2], 16) & 0x10  ):
            print("\t\t\t\t\tBit(4) : Quick Power Drop - Quick Power Drop/Oversubscription line asserted")
        else:
            print("\t\t\t\t\tBit(4) : Quick Power Drop - DISABLED")
        print("\t\t\t\t\tBit(3) : Reserved")
        if( int(mylist[3][:2], 16) & 0x04  ):
            print("\t\t\t\t\tBit(2) : Sync Request - OCC needs to restart snapshot buffers")
        else:
            print("\t\t\t\t\tBit(2) : Sync Request - NO snapshot sync needed")
        print("\t\t\t\t\tBit(1) : Reserved")
        if( int(mylist[3][:2], 16) & 0x01  ):
            print("\t\t\t\t\tBit(0) : Cooling Request - OCC has a fan increase request")
        else:
            print("\t\t\t\t\tBit(0) : Cooling Request - DISABLED")
        #######################################################################################################
        print("\tOCC Present (3)        : " + mylist[3][-2:] )
        #######################################################################################################
        configString = getConfigString(int(DataList[3],16));
        print("\tConfig Data Needed (4) : " + mylist[4][:2] + "\t" + configString)
        #######################################################################################################
        print("\tCurrent OCC State (5)  : " + mylist[4][-2:] + " : " ,end="")
        if(mylist[4][-2:] == "00"):
            print("\tReserved")
        elif(mylist[4][-2:] == "01"):
            print("\tStandby")
        elif(mylist[4][-2:] == "02"):
            print("\tObservation")
        elif(mylist[4][-2:] == "03"):
            print("\tActive")
        elif(mylist[4][-2:] == "04"):
            print("\tSafe")
        #######################################################################################################
        print("\tCurrent Power Mode (6) : " + mylist[5][:2])
        if(mylist[5][:2] == "00"):
            print("\t\t\t\t\tReserved")
        elif(mylist[5][:2] == "01"):
            print("\t\t\t\t\tNominal")
        elif(mylist[5][:2] == "02"):
            print("\t\t\t\t\tReserved")
        elif(mylist[5][:2] == "03"):
            print("\t\t\t\t\tTurbo (Static Turbo)")
        elif(mylist[5][:2] == "04"):
            print("\t\t\t\t\tSafe")
        elif(mylist[5][:2] == "05"):
            print("\t\t\t\t\tStatic Power Save")
        elif(mylist[5][:2] == "06"):
            print("\t\t\t\t\tDynamic Power Save (Favor Energy)")
        elif(mylist[5][:2] == "07"):
            print("\t\t\t\t\tMinimum Frequency")
        elif(mylist[5][:2] == "08"):
            print("\t\t\t\t\tCPM Turbo Frequency (NOT SUPPORTED)")
        elif(mylist[5][:2] == "09"):
            print("\t\t\t\t\tReserved")
        elif(mylist[5][:2] == "0A"):
            print("\t\t\t\t\tDynamic Power Save (FP)")
        elif(mylist[5][:2] == "0B"):
            print("\t\t\t\t\tFixed Requency Override (FFO)")
        elif(mylist[5][:2] == "0C"):
            print("\t\t\t\t\tFixed Maimum Frequency")
        #######################################################################################################
        print("\tCurrent Idle PWR SVR(7): " + mylist[5][-2:] + " : " + "{0:08b}".format( int(mylist[5][-2:], 16))   )
        print("\t\t\t\t\tBit(7:2): Reserved")
        if( int(mylist[5][-2:], 16) & 0x02  ):
            print("\t\t\t\t\tBit(1) : IDLE Power Saver - ACTIVE")
        else:
            print("\t\t\t\t\tBit(1) : IDLE Power Saver - IN-ACTIVE")
        if( int(mylist[5][-2:], 16) & 0x01  ):
            print("\t\t\t\t\tBit(0) : IDLE Power Saver - ENABLED")
        else:
            print("\t\t\t\t\tBit(0) : IDLE Power Saver - DISABLED")
        #######################################################################################################
        elogId = mylist[6][:2]
        print("\tError Log ID   : " + elogId)
        if(elogId == "00"):
            print("\t\t\t\t\tNO ERROR LOG")
        elogAddress = mylist[6][-2:] + mylist[7] + mylist[8][:2]
        print("\tError Log Start Address (9-12)  0x: " + elogAddress )
        elogLength = mylist[8][-2:] + mylist[9][2:]
        print("\tError Log Length (13-14) : 0x" + elogLength )
        print("\tReserved (15-16)  : " + mylist[9][-2:] + mylist[10][:2])
        print("\tChecksum : " + mylist[10][-2:] + mylist[11][:2])

        if(Successful != 2) and (SeqNum != '00'):
            hexDumpString(mylist)
            RC = 1

        if (elogId != "00"):
            print("\nReadPrint_CMD: OCC ERROR LOG WAS RETURNED! (id:0x"+elogId+", addr:0x"+elogAddress+", len=0x"+elogLength+")\n")
            ELOG_RC, elog = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x 0x"+elogAddress+" 0x"+elogLength, FLG_VRB)
            print("ReadSRAM returned rc "+str(ELOG_RC)+"\n")
            hexDumpString(elog)
            RC = 0xEEEE

    else:
        print("OCC Poll Validation - READ SRAM return code : " + str(RC) )
        print("or")
        print("Validation - CheckSum calculated (" + str(hex(CheckSum)) + ")     CheckSum from SRAM (" + str(hex(int(ReturnCheckSum, 16) ) ) + ")")
        ReadPrint_OCCreturn(FLG_VRB, '00')
        RC = 1

    print("RemoteReturnCode = " + str(RC))
    return RC


def ReadPrint_OCCreturn(FLG_VRB, SeqNum):
    RC = 0
    CheckSum = 0
    Successful = 0

    sramRC, mylist = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_rsp_buffer+" "+str(G_occ_rsp_buffer_size), FLG_VRB)

    if (mylist[0][:2] == SeqNum) and (int(mylist[1][:2],16) == 0xff):
        # Timeout waiting for response
        RC = 0xFD
    else:
        # loop through the data and calculate checksum.
        # sheldon9999 make checksum calculations more of algorithem than this.
        CheckSum += int( mylist[0][:2], 16 )        #seq number
        CheckSum += int( mylist[0][-2:], 16 )       #CMD
        CheckSum += int( mylist[1][:2], 16 )        #return Status
        loop1 = int(mylist[1][-2:] + mylist[2][:2],16)
        CheckSum += loop1                           #Data Length
        if(loop1 == 0):
            ReturnCheckSum = mylist[2][-2:] + mylist[3][:2]
        else:
            CheckSum += int( mylist[2][-2:], 16 )   #Data of 1, see above sheldon9999
            ReturnCheckSum = mylist[3][:2] + mylist[3][-2:]

        #CheckSum += 1       #sheldonTEST

        if(FLG_VRB):
            print("Calculated Checksum : " + str( hex( CheckSum ) ) + "\n\n")

    # SCRIPT DEFINED FAILURES
    # FC    Bad Checksum
    # FD    Timeout


        if( sramRC == None) and ( CheckSum == int(ReturnCheckSum, 16) ):
            print("Good Call Return from OCCread, and Good CheckSum " )
            print("======================================================")
            print("\tSeq Num                : " + mylist[0][:2],end="")
            #SeqNum = '10'                                 #sheldonTEST
            if(mylist[0][:2] == SeqNum):
                print("\tValidation - Matched Last CMD")
                Successful += 1
            elif(SeqNum == '00'):
                print("\tRead only no Validation.")
                Successful += 1
            else:
                print("\tValidation FAILED - Last CMD Sequence Number : " + SeqNum)
            cmdString=getCmdString(int(mylist[0][-2:],16))
            print("\tCommand                : " + mylist[0][-2:] + " : " + cmdString)
            rcString=getRcString(int(mylist[1][:2],16))
            print("\tReturn Status          : " + mylist[1][:2] + " : " + rcString)
            print("\tData Length            : " + mylist[1][-2:] + mylist[2][:2] )
            #######################################################################################################
            loop1 = int(mylist[1][-2:] + mylist[2][:2],16)
            if(loop1 == 0):
                print("\tCheckSum               : " + mylist[2][-2:] + mylist[3][:2])
            else:
                print("\tReturnCode             : " + mylist[2][-2:] )
                print("\tCheckSum               : " + mylist[3][:2] + mylist[3][-2:])



            if(Successful != 2) and (SeqNum != '00'):
                hexDumpString(mylist)
                RC = 1

        else:
            print("Validation - READ SRAM return code : " + str(RC) )
            print("or")
            print("Validation - CheckSum calculated (" + str(hex(CheckSum)) + ")     CheckSum from SRAM (" + str(hex(int(ReturnCheckSum, 16) ) ) + ")")
            hexDumpString(mylist)
            RC = 0xFC

    print("RemoteReturnCode = " + str(RC))
    return RC



def ReadPrint_OCCsensor(FLG_VRB, SeqNum):
    RC = 0
    CheckSum = 0
    Successful = 0
    last_sensor = 0

    sramRC, mylist = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_rsp_buffer+" "+str(G_occ_rsp_buffer_size), FLG_VRB)

    # SCRIPT DEFINED FAILURES
    # FC    Bad Checksum
    # FD    Timeout

    if (mylist[0][:2] == SeqNum) and (int(mylist[1][:2],16) == 0xff):
        # Timeout waiting for response
        RC = 0xFD
    else:
        CheckSum += int( mylist[0][:2], 16 )        #seq number
        CheckSum += int( mylist[0][-2:], 16 )       #CMD
        CheckSum += int( mylist[1][:2], 16 )        #return Status
        DataLength = int(mylist[1][-2:] + mylist[2][:2],16)
        CheckSum += int(mylist[1][-2:],16) + int(mylist[2][:2],16)

        DataList=[]
        if(DataLength != 0):
            LoopCount = 5
            # convert data to int array and continue checksum calculation
            for i in xrange(LoopCount, DataLength+LoopCount):
                LoopCount = i
                if(i%2==0):
                    DataList.append( int(mylist[ i/2 ][:2], 16 ))
                    CheckSum += int( mylist[ i/2 ][:2], 16 )
                else:
                    DataList.append( int(mylist[ i/2 ][-2:], 16 ))
                    CheckSum += int( mylist[ i/2 ][-2:], 16 )

            LoopCount += 1
            if( (LoopCount)%2==0):
                ReturnCheckSum = mylist[LoopCount/2][:2] + mylist[LoopCount/2][-2:]
            else:
                ReturnCheckSum = mylist[LoopCount/2][-2:] + mylist[(LoopCount/2)+1][:2]
        else:
            ReturnCheckSum = mylist[2][-2:] + mylist[3][:2]
        # Truncate checksum to 2 bytes
        CheckSum = CheckSum & int("FFFF",16)

        if(FLG_VRB):
            print("Calculated Checksum : " + str( hex( CheckSum ) ) + "\n\n")

        # Print Header
        print("\tSeq Num                : 0x" + mylist[0][:2],end="")
        if(mylist[0][:2] == SeqNum):
            print("\tValidation - Matched Last CMD")
            Successful += 1
        elif(SeqNum == '00'):
            print("\tRead only no Validation.")
            Successful += 1
        else:
            print("\tValidation FAILED - Last CMD Sequence Number : " + SeqNum)
        cmdString=getCmdString(int(mylist[0][-2:],16))
        print("\tCommand                : 0x" + mylist[0][-2:] + " : " + cmdString)
        rcString=getRcString(int(mylist[1][:2],16))
        print("\tReturn Status          : 0x" + mylist[1][:2] + " : " + rcString)
        print("\tData Length            : 0x" + '{:04X}'.format(DataLength) )

        if( sramRC == None) and ( CheckSum == int(ReturnCheckSum, 16) ):
            # Response for: OCCw -C 0x53 -D "05000000000002ffff" (0002 is the location (proc), ffff is the type)
            last_sensor = parseSensorList(DataList, DataLength, FLG_VRB)

            print("\tChecksum               : 0x" + ReturnCheckSum)

        else:
            print("ERROR: OCC Sensor Validation - READ SRAM return code : " + str(RC) )
            print("or")
            print("       CheckSum calculated (" + str(hex(CheckSum)) + ") vs CheckSum from SRAM (" + str(hex(int(ReturnCheckSum, 16) ) ) + ")")
            hexDumpString(mylist)
            RC = 0xFC

    print("RemoteReturnCode = " + str(RC))
    return last_sensor



def parseSensorList(DataList, DataLength, flg_verbose):
    last_sensor = 0

    # Response for: OCCw -C 0x53 -D "05000000000002ffff" (0002 is the location (proc), ffff is the type)
    truncated = DataList[0]
    print("\tList Truncated?    : " + str(truncated) + " (0 = full list, 1 = list truncated)")
    num_sensors = DataList[1]
    print("\tNum Sensors        : " + str(num_sensors))
    sensor_length=20
    print("\t\tGUID    Name              Sample")
    for i in xrange(0, num_sensors):
        offset = i * sensor_length
        last_sensor = (DataList[2+offset] << 8) + DataList[3+offset]
        print("\t\t0x" + '{:04X}'.format(last_sensor) + "  ", end='')
        for j in xrange(0, 16):
            if chr(DataList[4+offset+j]).isalnum():
                print(chr(DataList[4+offset+j]), end='')
            else:
                print(" ", end='')
        print("  0x" + '{:04X}'.format((DataList[20+offset] << 8)+DataList[21+offset]))

        # Check buffer overflow
        if (2 + offset + sensor_length) > DataLength:
            print("ERROR: Buffer size/Data length ("+str(DataLength)+") mismatch! current offset=" + str(2+offset))
            hexDumpInt(DataList);
            break

    # Print/Parse Data
    if flg_verbose:
        hexDumpInt(DataList);

    if truncated == 0:
        # no more sensors
        last_sensor = 0

    return last_sensor



def ValidateCmd(FLG_VRB, SeqNumExpected, FLG_LAST, flg_quiet):
    global L_pgpe_enabled

    RC = 0
    CheckSum = 0
    Successful = 0
    CheckPoint = 0
    delay = 1

    if L_pgpe_enabled == 1:
        delay = 4

    if (FLG_LAST):
        print("OCCreadvalidate: Waiting for seq 0x" + SeqNumExpected + " (last try)")
    else:
        if not flg_quiet:
            print("OCCreadvalidate: Waiting for seq 0x" + SeqNumExpected)

    # retry does not work because simics does NOT continue to execute while this
    # script is running...  Have to return timeout to caller and then they call again
    for retry in range(1):
        if FLG_VRB:
            print("==> backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_rsp_buffer+" "+str(G_occ_rsp_buffer_size))
        sramRC, mylist = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_rsp_buffer+" "+str(G_occ_rsp_buffer_size), FLG_VRB)

        SeqNumberRcvd = str( format( int(mylist[0][:2], 16),'02X') )    #seq number
        LastSeq = str('{0:02X}'.format(int(SeqNumExpected, 16)-1))

        CmdNumber=int(mylist[0][-2:],16)
        cmdString=getCmdString(CmdNumber)

        RtnCode = int(mylist[1][:2],16); # Convert to integer
        rcString=getRcString(RtnCode)

        DataLength = int(mylist[1][-2:] + mylist[2][:2],16)

        CheckPoint = int(mylist[3], 16)
        if (SeqNumExpected == "00") and (RtnCode == 0xE1):
            if (CheckPoint == 0x0EFF):
                print("ValidateCmd: Initialization Checkpoint 0x%04X (COMPLETE)" % CheckPoint + " (data length="+str(DataLength)+")")
            else:
                print("ValidateCmd: Initialization Checkpoint 0x%04X" % CheckPoint + " (data length="+str(DataLength)+")")
#            hexDumpInt([int(mylist[2][-2:],16),int(mylist[3][:2],16),int(mylist[3][-2:],16),int(mylist[4][:2],16),
#                int(mylist[4][-2:],16),int(mylist[5][:2],16),int(mylist[5][-2:],16),int(mylist[6][:2],16)])

        #print("DEBUG: SeqNumExpected="+SeqNumExpected+", RtnCode=0x%02X"%RtnCode+", CheckPoint=0x%04X"%CheckPoint)
        if (RtnCode == 0xFF):
            print("ValidateCmd: " + cmdString + " command (0x%02X)" % CmdNumber + \
                    ", seq 0x" + str(SeqNumExpected) + " - still processing")
        elif ((SeqNumExpected != "00") and (RtnCode == 0xE1) and (CheckPoint == 0x0EFF)):
            print("ValidateCmd: " + cmdString + " command (0x%02X)" % CmdNumber + \
                    ", seq 0x" + str(SeqNumExpected) + " - NOT ACKNOWLEDGED (still has COMM INIT CHECKPOINT)")
            # Response length = 3 rsp header + 2 (data length bytes) + Data Length
            half_words = ((5+DataLength) / 2) + 1;
            # Truncate the buffer to only include the response and rsp data
            del mylist[half_words:]
            hexDumpString(mylist)
            RtnCode = 0xFF
        elif (SeqNumberRcvd == LastSeq):
            print("ValidateCmd: " + cmdString + " command (0x%02X)" % CmdNumber + \
                    ", seq 0x" + str(SeqNumExpected) + " - OCC didnt ack command (old Seq#) - still processing???")
        else:
            print("ValidateCmd: " + cmdString + " command (0x%02X)" % CmdNumber + \
                    ", seq 0x" + str(SeqNumExpected) + " returned 0x%02X" % RtnCode + " " + rcString)
            break
        time.sleep(delay)
    else:
        if ( not ((RtnCode == 0xE0) or ((RtnCode > 0xE1) and (RtnCode <= 0xEF))) ):
            # just a timeout on the command
            if not flg_quiet or FLG_LAST:
                print("ValidateCmd: Timeout waiting for OCC response seq 0x"+SeqNumExpected)
            RC = 0xFD
        #else OCC Critical Error

    if (RC != 0xFD):
        # Calculate the response checksum (for validation)
        CheckSum += int( SeqNumberRcvd, 16 )
        CheckSum += CmdNumber
        CheckSum += RtnCode
        #CheckSum += DataLength
        CheckSum += int( mylist[1][-2:], 16)
        CheckSum += int( mylist[2][:2], 16)

        DataList=[]
        if(DataLength != 0):
            LoopCount = 5
            for i in xrange(LoopCount, DataLength+LoopCount):
                LoopCount = i
                #print("THis is the counter loop : " + str(LoopCount))
                if(i%2==0):
                    #print("even number ->" + str( i ) + " / " + str( i/2 ) + " -2: " + mylist[ i/2 ][:2] )
                        DataList.append( int(mylist[ i/2 ][:2], 16 ))
                        CheckSum += int( mylist[ i/2 ][:2], 16 )
                else:
                    #print("odd number ->" + str(i) + " / " + str( i/2 ) + " :2 " + mylist[ i/2 ][-2:] )
                        DataList.append( int(mylist[ i/2 ][-2:], 16 ))
                        CheckSum += int( mylist[ i/2 ][-2:], 16 )

            LoopCount += 1                      #srb01a
            if( (LoopCount)%2==0):              #srb01r LoopCount+1
                #print("even number ->" + str( LoopCount ) + " / " + str( LoopCount/2 ) + " :2 " + mylist[ LoopCount/2 ][:2] )
                ReturnCheckSum = mylist[LoopCount/2][:2] + mylist[LoopCount/2][-2:]
            else:
                #print("odd number ->" + str( LoopCount ) + " / " + str( LoopCount/2 ) + " -2: " + mylist[ LoopCount/2 ][-2:] )
                ReturnCheckSum = mylist[LoopCount/2][-2:] + mylist[(LoopCount/2)+1][:2]
        else:
            ReturnCheckSum = mylist[2][-2:] + mylist[3][:2]
        # Truncate checksum to 2 bytes
        CheckSum = CheckSum & int("FFFF",16)

    if (RC == 0xFD):
    #    # cmd timeout
        RC = RC
    elif (sramRC != None):
        # if RC == None then throw an error getting SRAM data.
        print("\n\n############################################################")
        print("ERROR: ReadSRAM("+G_occsram_rsp_buffer+","+str(G_occ_rsp_buffer_size)+") failed!")
        RC = 1

    # if Return Status is E0-EF Critical OCC error
    elif (RtnCode >= 0xE0) and (RtnCode <= 0xEF):
        if (RtnCode != 0xE1):
            print("ERROR: OCC hit critical error " + "0x%02X" % RtnCode + " : " + rcString)
            RC = RtnCode
        else: # 0xE1 is Initialization Checkpoint
            # Response buffer contains initialization checkpoint - OCC has not acknowledged any command yet
            print("OCC Checkpoint: 0x%04X" % CheckPoint)
            if ((SeqNumberRcvd == "00") and (CheckPoint == 0x0EFF)):
                # Final initialization checkpoint in buffer
                RC = RtnCode
            else:
                RC = 0xFF

    # if CheckSum != ReturnCheckSum Display Error and call print raw data.
    elif( CheckSum != int(ReturnCheckSum, 16) ):
        print("\n\n############################################################")
        print("ERROR: CheckSum calculated (" + str(hex(CheckSum)) + ")   !=   CheckSum from SRAM (" + str(hex(int(ReturnCheckSum, 16) ) ) + ")")
        RC = 0xFC

    # if Last Command Seq. number does not match the read SRAM return Seq. number then Display Error and call print raw data.
    elif(SeqNumberRcvd != SeqNumExpected):
        last_expected = '%02X' % ( int(SeqNumExpected,16) - 1 )
        if(SeqNumberRcvd == last_expected):
            print("\n\n############################################################")
            print("ERROR: SRAM Response buffer still had last SeqNumber (" + SeqNumRvcd + ") vs expected (" + SeqNumberExpected + ")")
            RC = 1
        elif(SeqNumberRcvd != 0) and (SeqNumExpected == "00"):
            print("IGNORING SeqNumber mismatch (received " + SeqNumberRcvd + ")")
        else:
            print("\n\n############################################################")
            print("ERROR: SeqNumber You Expected (" + SeqNumExpected + ")   !=   SeqNumber from SRAM (" + SeqNumberRcvd + ")")
            RC = 1

    # if Return Status != "00" or Success Display Error and call print that Error and raw data.
    elif (RtnCode != 0):
        print("======================================================")
        print("\tSeq Num                : 0x" + SeqNumberRcvd )
        print("\tCommand                : " + "0x%02X" % CmdNumber + " : " + cmdString)
        print("\tReturn Status          : " + "0x%02X" % RtnCode + " : " + rcString)
        print("\tData Length            : " + "0x%04X" % DataLength )
        print("ERROR: OCC returned status 0x%02X" % RtnCode + " : " + rcString)
        RC = 1

    if (RC):
        if (RC == 0xE0):
            # Parse OCC Panic
            hexDumpPanic(mylist)
        elif (RC >= 0xE0) and (RC <= 0xEF):
            # Dump exception buffer
            hexDumpString(mylist)
        elif (RC != 0xFD) or (FLG_LAST):
            # dont dump buffer on timeout, unless last
            if (RtnCode >= 0x10):
                # Response length = 3 rsp header + 2 (data length bytes) + Data Length
                half_words = ((5+DataLength) / 2) + 1;
                # Truncate the buffer to only include the response and rsp data
                del mylist[half_words:]
            print("\n")
            hexDumpString(mylist)
            print("############################################################\n\n")
    else:
        # GETTING HERE MEANS WE HAVE A SUCCESSFUL COMMAND.
        print("======================================================")
        print("\tSeq Num                : 0x" + SeqNumberRcvd )
        print("\tCommand                : " + "0x%02X" % CmdNumber + " : " + cmdString)
        print("\tReturn Status          : " + "0x%02X" % RtnCode + " : " + rcString)
        print("\tData Length            : " + "0x%04X" % DataLength )
        if (DataLength > 0):
        #if (1 == 1):
            ### PARSE POLL RESPONSE ###
            if(CmdNumber == 0):
                #######################################################################################################
                statusBits=["Master","Collect Fir Data","","OCC owns PMCR","Simics","","Observation Ready","Active Ready"]
                values = parseBitField(DataList[0], statusBits)
                print("\tStatus (1)             : " + "0x%02X" % DataList[0] + " : " + values)
                #######################################################################################################
                extStatusBits=["DVFS due to Proc OT","DVFS due to Power","Memory Trottle due to OT","Quick Power Drop","DVFS due to Vdd OT"]
                values = parseBitField(DataList[1], extStatusBits)
                print("\tExt Status (2)         : " + "0x%02X" % DataList[1] + " : " + values)
                #######################################################################################################
                print("\tOCC Present (3)        : " + "0x%02X" % DataList[2] + " : {0:08b}".format(DataList[2]))
                #######################################################################################################
                CurrentString = str(   format(DataList[3],'02X')  )
                configString = getConfigString(DataList[3]);
                print("\tConfig Data Needed (4) : 0x" + CurrentString + " : " + configString)
                #######################################################################################################
                print("\tCurrent OCC State (5)  : " + "0x%02X" % DataList[4] + " : ", end="")
                if (DataList[4] == 0x00): print("N/A")
                elif(DataList[4] == 0x01): print("Standby")
                elif(DataList[4] == 0x02): print("Observation")
                elif(DataList[4] == 0x03): print("Active")
                elif(DataList[4] == 0x04): print("Safe")
                else: print("UNKNOWN")
                #######################################################################################################
                print("\tCurrent Power Mode (6) : " + "0x%02X" % DataList[5] + " : " ,end="")
                if(DataList[5] == "00"):   print("(O/S Frequency Control)")
                elif(DataList[5] == 0x01): print("Nominal")
                elif(DataList[5] == 0x03): print("Turbo")
                elif(DataList[5] == 0x04): print("Safe")
                elif(DataList[5] == 0x05): print("Static Power Save")
                elif(DataList[5] == 0x0A): print("Dynamic Performance")
                elif(DataList[5] == 0x0B): print("Fixed Frequency Override")
                elif(DataList[5] == 0x0C): print("Maximum Performance")
                else: print("UNKNOWN")
                #######################################################################################################
                if (DataList[6] == 0):
                    values = "Disabled"
                else:
                    ipsStatusBits=["","","","","","","Active","Enabled"]
                    values = parseBitField(DataList[6], ipsStatusBits)
                print("\tIdle Power Saver (7)   : " + "0x%02X" % DataList[6] + " : " + values)
                #######################################################################################################
                elogId = str(   format(DataList[7],'02X')  )
                print("\tError Log ID (8)       : 0x" + elogId ,end="")
                if (elogId == "00"): print("   (no error log)")
                else: print("")
                elogAddress = str(format(DataList[8],'02X')) + str(format(DataList[9],'02X')) + str(format(DataList[10],'02X')) + str(format(DataList[11],'02X'))
                print("\tError Log Addr (9-12)  : 0x" + elogAddress)
                elogLength = str(format(DataList[12],'02X')) + str(format(DataList[13],'02X'))
                print("\tError Log Length(13-14): 0x" + elogLength)
                print("\tError Log Source (15)  : " + "0x%02X" % DataList[14] + " : ", end="")
                if   (DataList[14] == 0x00): print("OCC 405")
                elif (DataList[14] == 0x10): print("PGPE")
                elif (DataList[14] == 0x20): print("SGPE")
                else                       : print("UNKNOWN")
                #######################################################################################################
                if (DataList[15] == 0):
                    values = "None"
                else:
                    gpuPresentBits=["","","","","","GPU2","GPU1","GPU0"]
                    values = parseBitField(DataList[15], gpuPresentBits)
                print("\tGPU Configuration (16) : " + "0x%02X" % DataList[15] + " : {0:08b}".format(DataList[15]) + " : " + values)
                #######################################################################################################
                print("\t##############################################################")
                hexstring = ''
                for i in xrange(16,32):
                    hexstring += str(format(DataList[i],'02X'))
                print("\tOCC Code Level(17-32)  : " + bytearray.fromhex(hexstring).decode() )
                hexstring = ''
                for i in xrange(32,38):
                    hexstring += str(format(DataList[i],'02X'))
                print("\tSENSOR (33-38)         : " + bytearray.fromhex(hexstring).decode() )
                print("\t##############################################################")
                print("\tNum. Sensor Blocks (39): 0x" + str(format(DataList[38],'02X')) )
                print("\tSensor Version (40)    : 0x" + str(format(DataList[39],'02X')) )
                print("\t##############################################################")
                indexdata = 40
                for j in xrange(0,DataList[38] ):   # Loop through all the Sensor Data Blocks.
                    hexstring = ''
                    for i in xrange(0,4):   # 4 bytes Eye Catcher
                        hexstring += str(format(DataList[indexdata],'02X'))
                        indexdata += 1
                    eyecatcher = bytearray.fromhex(hexstring).decode()
                    print("\tSensor Name (0-3)      : " + eyecatcher )
                    print("\tReserved (4)           : 0x" + str(format(DataList[indexdata],'02X')) )  # 1 byte Reserved
                    indexdata += 1
                    print("\tSensor format (5)      : 0x" + str(format(DataList[indexdata],'02X')) )  # 1 byte sensor format
                    indexdata += 1
                    sensorlength = DataList[indexdata]
                    indexdata += 1
                    print("\tSensor Length (6)      : 0x" + str(format(sensorlength ,'02X')) )  # 1 byte Sensor Length
                    numbersensors = DataList[indexdata]
                    indexdata += 1
                    print("\tNumber Sensors (7)     : 0x" + str(format(numbersensors,'02X')) )   # 1 byte Number Sensors
                    if eyecatcher == "CAPS":
                        print("\t\tSensorData     :  " ,end="")
                        for k in xrange(0, sensorlength):   # loop through data in Sensor
                            if ((k > 0) and ((k % 24) == 0)): # split hex data on lines of 24 bytes
                                print("\n\t\t               :  " ,end="")
                            print(str(format(DataList[indexdata+k],'02X'))+" ",end="")
                        print("");
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t      Current Power Cap: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")  (output power)")
                        indexdata += 2
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t          Current Power: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")")
                        indexdata += 2
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t            N Power Cap: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")")
                        indexdata += 2
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t          Max Power Cap: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")")
                        indexdata += 2
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t     Hard Min Power Cap: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")")
                        indexdata += 2
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t     Soft Min Power Cap: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")")
                        indexdata += 2
                        value = (DataList[indexdata] << 8) + DataList[indexdata+1]
                        print("\t\t         User Power Cap: " + '{:6d}'.format(value) + " Watts (0x"+ str(format(value,'04X')) + ")")
                        indexdata += 2
                        print("\t\tUser Power Limit Source: " + str(DataList[indexdata]) + "  (1=TMGT/BMC, 2=OPAL)")
                        indexdata += 1
                    else:
                        for i in xrange(0, numbersensors):   # loop through the number of Sensors
                            print("\t\tSensorData     :  " ,end="")
                            for k in xrange(0, sensorlength):   # loop through data in Sensor
                                if ((k > 0) and ((k % 24) == 0)): # split hex data on lines of 24 bytes
                                    print("\n\t\t               :  " ,end="")
                                print(str(format(DataList[indexdata],'02X'))+" ",end="")
                                indexdata += 1
                            if (eyecatcher == "EXTN"): # print eyecatchers for EXTN data
                                print("  \"",end="")
                                tempindex = indexdata - sensorlength
                                for k in xrange(0, 4):
                                    print(chr(DataList[tempindex]),end="")
                                    tempindex += 1
                                print("\"",end="")
                            print("")
                    print("\t##############################################################")

                if (elogId != "00"):
                    print("\nOCC ERROR LOG WAS RETURNED! (id:0x"+elogId+", addr:0x"+elogAddress+", len=0x"+elogLength+")\n")
                    ELOG_RC, elog = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x 0x"+elogAddress+" 0x"+elogLength, FLG_VRB)
                    print("OCC SRC: 0x2A"+elog[2][:2].upper()+" / 0x"+elog[4][-2:]+elog[5][:2]+", Severity:0x"+elog[2][-2:]+", Actions:0x"+elog[3][:2]+"\n")
                    #hexDumpString(elog)
            else:
                # Not a poll command (and it has rsp data)
                if (len(DataList) > 0):
                    hexDumpInt(DataList);
            print("\tCheckSum               : 0x" + ReturnCheckSum )
        else:
            if (SeqNumExpected == "00") and (CmdNumber == 0):
                print("\n\n############################################################")
                print("ERROR: Response buffer may be uninitialized.  Check if OCC was started")
                RC = 2

    print("RemoteReturnCode = " + str(RC))
    print("RemoteReturnCode = " + "0x%02X" % RC)

    return RC


#===============================================================================
#
# write_cmd_func
#
# Main function for the command.  This is the entry point for the command line
# invocation of the 'example_cmd' command.
#
#===============================================================================
def write_cmd_func(flg_Cmd, Cmd_arg, flg_Data, Data_arg, flg_verbose ):
    """write_command function"""
    Parms = 0
    RCSeqNum = 0

    if flg_Cmd: # Parse Command: -C xxx
        Command = '{0:02X}'.format(Cmd_arg)
        AcceptableCMD = {   '00' : 1,     #OCC_CMD_POLL
                            '01' : 1,     #OCC_CMD_QUERY_FIRMWARE_LEVEL

                            '10' : 1,     #OCC_CMD_GET_ERROR_LOG
                            '11' : 1,     #OCC_CMD_CONTINUE_ERROR_LOG
                            '12' : 1,     #OCC_CMD_CLEAR_ERROR_LOG

                            '20' : 1,     #OCC_CMD_SET_MODE_AND_STATE
                            '21' : 1,     #OCC_CMD_SETUP_CONFIGURATION_DATA
                            '22' : 1,     #OCC_CMD_SET_POWER_LIMIT
                            '23' : 1,     #OCC_CMD_SET_THERMAL_THROTTLE_MODE
                            '24' : 1,     #OCC_CMD_SET_PEX_REGISTER
                            '25' : 1,     #OCC_CMD_RESET_PREP
                            '26' : 1,     #OCC_CMD_SNAPSHOT_SYNC

                            '30' : 1,     #OCC_CMD_PEX_PASS_THROUGH
                            '31' : 1,     #OCC_CMD_READ_STATUS_REGISTER
                            '32' : 1,     #OCC_CMD_GET_THROTTLE_VALUES
                            '33' : 1,     #OCC_CMD_GET_CPU_TEMPERATURES
                            '34' : 1,     #OCC_CMD_GET_COOLING_REQUEST
                            '35' : 1,     #OCC_CMD_GET_OCC_SNAPSHOT

                            '40' : 1,     #OCC_CMD_DEBUG_PASS_THROUGH
                            '41' : 1,     #OCC_CMD_AME_PASS_THROUGH
                            '42' : 1,     #OCC_CMD_GET_FIELD_DEBUG_DATA

                            '50' : 1,     #OCC_CMD_VRM_INTERFACE_TEST
                            '51' : 1,     #OCC_CMD_PROCESSOR_INTERFACE_TEST
                            '52' : 1,     #OCC_CMD_SET_PROCESSOR_CLOCK_FREQ
                            '53' : 1,     #OCC_CMD_MFG_TEST
                            '54' : 1,     #OCC_CMD_GET_FOM_DATA

                            '60' : 1,     #OCC_CMD_TUNABLE_PARAMETERS

                            #'FE' : 0,     #OCC_CMD_INTERNAL_SERVER_CMD
                        }
        if( AcceptableCMD.get(Command, "nothing") == 0 ):
            helper("Unsupported Command: -C " + Command )
        elif( AcceptableCMD.get(Command, "nothing") == 1 ):
            if flg_verbose:
                print("Supported  Command : -C " + Command)
            Parms += 1
        else:
            print("WARNING: Unrecognized Command : -C " + Command)
            Parms += 1
            helper("Unrecognized Command : -C " + Command)

    if flg_Data: # Parse Data: -D xxx
        ThisTuple = []
        # Break String up into a Byte list.
        n=2
        ByteList =  [ Data_arg[i:i+n] for i in range (0, len(Data_arg), n)]
        ByteCount = len(ByteList)

        if flg_verbose:
            print("Command Data : -D " + Data_arg )
            print("Byte Count( " + str(ByteCount) + ") : ",end="")
            print(ByteList)

        Parms += 1

    if(Parms == 2): # Send command
        Address2Write = int(G_occsram_cmd_buffer,16)

        global LastCmd_SeqNum
        # Start with sequence number
        #   random sequence number:
        #RCSeqNum = random.randint(1, 255)
        #   incrementing sequence number (like TMGT):
        RCSeqNum = LastCmd_SeqNum+1
        LastCmd_SeqNum = RCSeqNum

        cmdString=getCmdString(Cmd_arg)
        print("OCCwrite: Sending " + cmdString + "(0x"+str(Command) + ") command (" + str(ByteCount) + " data bytes, seq 0x"+ str('{0:02X}'.format(RCSeqNum)) + ")")

        #(1) Setup the PIB to write to the OCBCSR1 register
        #command = "OccStandalone0.PowmanOccSlot.pib_master->address = 0x6d033"
        #print(command)
        #run_command(command)

        #(2) Set the channel to circular mode
        #command = "OccStandalone0.PowmanOccSlot.pib_master->data = 0x0c00000000000000"
        command = "backplane0."+proc+".pib_cmp.pib.write address = 0x6d0330 0x0c00000000000000 size = 8"
        if flg_verbose:
            print("==> "+command)
        run_command(command)

        # build the command string (while doing simple addition checksum)
        CmdBuffer = str('{0:02X}'.format(RCSeqNum))
        checksum2 = RCSeqNum
        CmdLength = 1

        # put passed in COMMAND into CMD buffer, and add to checksum
        checksum2 += Cmd_arg
        CmdBuffer += Command
        CmdLength += 1

        # put length of data command into CMD buffer, and add to checksum.
        checksum2 += (len(ByteList) >> 8) + (len(ByteList) & 0xFF)
        CmdBuffer += str( '{0:04X}'.format(len(ByteList))   )
        CmdLength += 2


        for indexNow in range(0, ByteCount):
#            print( " LOOP " + str(indexNow) + " : " + str(ByteCount))
            if CmdLength == 8:
#                print(str(indexNow) + " : SEND ")
                #(3) This is the command we're sending to the OCC
                command = "backplane0."+proc+".occ_cmp.oci_space.write " + str(hex(Address2Write)) + " 0x" + CmdBuffer + \
                    " size = " + str(len(CmdBuffer)/2)
                if indexNow == 0 or flg_verbose:
                    print("==> "+command)
                run_command(command)

                # increment address by 8 bytes for next command send.
                Address2Write += 8
                # Clear the buffer and set size to zero to build the next command.
                CmdBuffer = ""
                CmdLength = 0

            if CmdLength < 8 :
#                print(str(indexNow) + " : APPEND ")
                CmdBuffer += str( '{0:02X}'.format( int(ByteList[indexNow], 16)) )
                checksum2 += long(ByteList[indexNow], 16)
                CmdLength += 1

        if CmdLength < 7 :
            # put checksum into CMD buffer.
            CmdBuffer += str( '{0:04X}'.format(checksum2)       )
            #(3) This is the command we're sending to the OCC (this is a poll command)
            command = "backplane0."+proc+".occ_cmp.oci_space.write " + str(hex(Address2Write)) + " 0x" + CmdBuffer + \
                    " size = " + str(len(CmdBuffer)/2)
            print(command)
            run_command(command)
            #DONE DONE DONE
        elif CmdLength == 7 :
            # put FIRST 1/2 of checksum Number into CMD buffer.
            CmdBuffer += str( '{0:04X}'.format(checksum2)       )[:2]
            #(3) This is the command we're sending to the OCC (this is a poll command)
            command = "backplane0."+proc+".occ_cmp.oci_space.write " + str(hex(Address2Write)) + " 0x" + CmdBuffer + \
                    " size = " + str(len(CmdBuffer)/2)
            print(command)
            run_command(command)

            # increment address by 8 bytes for next command send.
            Address2Write += 8
            # Clear the buffer and set size to zero to build the next command.
            CmdBuffer = ""
            CmdLength = 0

            # put SECOND 1/2 of checksum Number into CMD buffer.
            CmdBuffer += str( '{0:04X}'.format(checksum2)       )[-2:]
            #(3) This is the command we're sending to the OCC (this is a poll command)
            command = "backplane0."+proc+".occ_cmp.oci_space.write " + str(hex(Address2Write)) + " 0x" + CmdBuffer + \
                    " size = " + str(len(CmdBuffer)/2)
            if flg_verbose:
                print("==> "+command)
            run_command(command)
            #DONE DONE DONE
        else:   # This is 8 CMD full send and then send checksum.
            #(3) This is the command we're sending to the OCC
            command = "backplane0."+proc+".occ_cmp.oci_space.write " + str(hex(Address2Write)) + " 0x" + CmdBuffer + \
                    " size = " + str(len(CmdBuffer)/2)
            print("==> "+command)
            run_command(command)
            # increment address by 8 bytes for next command send.
            Address2Write += 8
            # Clear the buffer and set size to zero to build the next command.
            CmdBuffer = ""
            CmdLength = 0
            # put checksum into CMD buffer.
            CmdBuffer += str('{0:04X}'.format(checksum2))
            #(3) This is the command we're sending to the OCC (this is a poll command)
            command = "backplane0."+proc+".occ_cmp.oci_space.write " + str(hex(Address2Write)) + " 0x" + CmdBuffer + \
                    " size = " + str(len(CmdBuffer)/2)
            if flg_verbose:
                print("==> "+command)
            run_command(command)
            #DONE DONE DONE


        #(4) Set the PIB to write to the OCBDR1 register
        #command = "OccStandalone0.PowmanOccSlot.pib_master->address = 0x6d035"
        #print(command)
        #run_command(command)

        #(5) Data that tells the OCC it should read the command buffer
        #command = "OccStandalone0.PowmanOccSlot.pib_master->data = 0x0101000000000000 "
        command = "backplane0."+proc+".pib_cmp.pib.write address = 0x6d0350 0x0101000000000000 size = 8"
        if flg_verbose:
            print("==> "+command)
        run_command(command)

        if flg_verbose:
            #print("OCC Command Data:")
            #hexDumpByteString(ByteList)
            ReadPrint_CMD(flg_verbose, 4+ByteCount+2)


    return cli.command_return( message = "return Sequence Number(%s)" % str('{0:02X}'.format(RCSeqNum)),
                               value = RCSeqNum)



#===============================================================================
#
# read_cmd_func
#
# Main function for the command.  This is the entry point for the command line
# invocation of the 'example_cmd' command.
#
#===============================================================================
def read_cmd_func(flg_Cmd, Cmd_arg, flg_verbose ):
    """read_command function"""
    if flg_Cmd:
        if( '{0:02X}'.format(Cmd_arg) == '00'):
            ReadPrint_POLL(flg_verbose, '00')
        elif( '{0:02X}'.format(Cmd_arg) == '53'):
            ReadPrint_OCCsensor(flg_verbose, '00')
        else:
            ReadPrint_OCCreturn(flg_verbose, '00')

    return


#===============================================================================
#
# rValidate_cmd_func
#
# Main function for the command.  This is the entry point for the command line
# invocation of the 'example_cmd' command.
#
#===============================================================================
def rValidate_cmd_func(flg_Cmd, Cmd_arg, flg_Data, Data_arg, flg_verbose, flg_last, flg_quiet):
    """validate_command function"""
    RC = 0
    if flg_Cmd:
        if flg_verbose:
            print("Sequence Number from last CMD : " + str('{0:02X}'.format(Data_arg)))

        RC = ValidateCmd(flg_verbose, str('{0:02X}'.format(Data_arg)), flg_last, flg_quiet )

    return RC



#===============================================================================
#
# occ_trace
#
# Main function for the command.  This is the entry point for the command line
# invocation of the 'example_cmd' command.
#
#===============================================================================
def occ_trace(directory_arg, flg_verbose, flg_directory, i_directory, flg_prefix, i_prefix):
    """Dump OCC trace"""
    RC = 0
    OCC = 0

    # Determine where the string file is located
    parse_opts = ""
    if directory_arg:
        if os.path.isdir(directory_arg):
            if os.path.isfile(directory_arg+"/occStringFile"):
                print("Using occStringFile from: "+directory_arg+"/")
                parse_opts = "-t "+directory_arg+"/occStringFile"
            else:
                print("Using occStringFile from OCC repo: "+directory_arg)
                parse_opts = "-g "+directory_arg
        elif os.path.isfile(directory_arg):
                print("Using string file: "+directory_arg)
                parse_opts = "-t "+directory_arg
        else:
            if not flg_directory and not flg_prefix:
                print("WARNING: IGNORING argument: "+directory_arg+" (NOT a directory or a file)");
    if parse_opts == "":
        occrepo = os.environ.get('OCCREPO')
        if (occrepo != ""):
            print("OCCREPO was defined so using occStringFile from: "+occrepo)
            parse_opts="-g "+occrepo

    # Determine where to write the trace files
    target_trace_directory = ""
    print("DEBUG: occ_trace: current directory: "+os.getcwd())
    if flg_directory:
        if i_directory:
            if os.path.isdir(i_directory):
                target_trace_directory = i_directory
            else:
                print("WARNING: Invalid target directory specified with -D option")
        else:
            print("WARNING: -D parameter requires a target directory")

    if not target_trace_directory:
        target_trace_directory = os.getcwd()+"/traces"
        if not os.path.isdir(target_trace_directory):
            target_trace_directory = os.getcwd()

    print("Dumping OCC trace for OCC" + str(OCC))
    print("Trace data will be written to: "+target_trace_directory)

    # Determine what prefix should get added (if any)
    saved_trace_name=""
    if flg_prefix:
        if i_prefix:
            saved_trace_name = target_trace_directory + "/" + i_prefix + "_occMERG.txt"
            print("Trace will be saved in: "+saved_trace_name)
        else:
            print("WARNING: -P parameter specified but no prefix was given")

    # Cleanup prior files
    command = "!rm -f "+target_trace_directory+"/*.bin "+target_trace_directory+"/*.gpebin"
    run_command(command);

    trace_filename = target_trace_directory+"/traceinf.bin"
    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_inf_trace+" "+G_occsram_trace_length+"\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting INF traces: " + command)
    run_command(command)

    trace_filename = target_trace_directory+"/traceimp.bin"
    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_imp_trace+" "+G_occsram_trace_length+"\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting IMP traces: " + command)
    run_command(command)

    trace_filename = target_trace_directory+"/traceerr.bin"
    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_err_trace+" "+G_occsram_trace_length+"\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting ERR traces: " + command)
    run_command(command)

    trace_filename = target_trace_directory+"/tracegpe0.gpebin"
    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x (backplane0."+proc+".occ_cmp.oci_space.read 0xfff01184 4) (backplane0."+proc+".occ_cmp.oci_space.read 0xfff01188 4)\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting GPE0 traces: " + command)
    run_command(command)

    trace_filename = target_trace_directory+"/tracegpe1.gpebin"
    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x (backplane0."+proc+".occ_cmp.oci_space.read 0xfff10184 4) (backplane0."+proc+".occ_cmp.oci_space.read 0xfff10188 4)\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting GPE1 traces: " + command)
    run_command(command)

    trace_filename = target_trace_directory+"/tracessx.gpebin"
    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x (backplane0.dcm0.chip0.occ_cmp.proc_405.sym g_ssx_trace_buf_ptr) 512\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting SSX traces: " + command)
    run_command(command)

    # Parse the trace files
    command = "!"+tools_directory+"/parseOccBinTrace " + parse_opts + " " + target_trace_directory

    if saved_trace_name:
        command = command + " > " + saved_trace_name
        command2 = "!rm -f "+saved_trace_name # Cleanup prior file
        run_command(command2);

    print("==> Parsing OCC traces: " + command + "\n")
    run_command(command)

    if saved_trace_name:
        command = "!cat " + saved_trace_name
        run_command(command)

    return RC


#===============================================================================
#
# Collect and parse the PGEP Trace
#
#===============================================================================
def pgpe_trace(directory_arg, flg_verbose):
    """Dump PGPE trace"""
    RC = 0
    OCC = 0

    print("Dumping PGPE trace for OCC" + str(OCC))

    # Determine where to write traces (should be different from OCC traces so it does not try to parse them together)
    target_trace_directory = os.getcwd()+"/traces"
    if not os.path.isdir(target_trace_directory):
        target_trace_directory = os.getcwd()
    target_trace_directory = target_trace_directory + "/pgpe"
    if not os.path.isdir(target_trace_directory):
        os.mkdir(target_trace_directory)
    print("Traces will be written to: "+target_trace_directory)

    trace_address_ptr = "0xfff20204"
    trace_length_ptr = "0xfff20208"
    trace_filename = target_trace_directory+"/tracegpe2.gpebin"

    command = "pipe \"backplane0."+proc+".occ_cmp.oci_space.x (backplane0."+proc+".occ_cmp.oci_space.read "+trace_address_ptr+" 4) (backplane0."+proc+".occ_cmp.oci_space.read "+trace_length_ptr+" 4)\" \"sed \\\"s/^p:0x........ //g\\\" | sed \\\"s/ ................$//g\\\" | sed \\\"s/ //g\\\" | xxd -r -p > "+trace_filename+" \""
    print("==> Collecting PGPE (GPE2) traces: " + command)
    run_command(command)

    # Determine where the string files is located
    parse_opts = ""
    if directory_arg:
        if os.path.isdir(directory_arg):
            if os.path.isfile(directory_arg+"/trexStringFile"):
                print("Using string file from: "+directory_arg+"/")
                string_name = directory_arg+"/trexStringFile"
                parse_opts = "-s "+string_name
            else:
                print("Using string file from OCC repo: "+directory_arg)
                parse_opts = "-g "+directory_arg
        elif os.path.isfile(directory_arg):
            print("Using string file: "+directory_arg)
            string_name = directory_arg
            parse_opts = "-s "+string_name
        else:
            print("WARNING: IGNORING argument: "+directory_arg+" (NOT a directory or a file)");
    else:
        if os.path.isfile("binaries/trexStringFile"):
            parse_opts = "-s binaries/trexStringFile"

    # Parse the trace file
    command = "!"+tools_directory+"/ppe-trace.exe "+trace_filename+" > "+target_trace_directory+"/tracegpe2.bin "+parse_opts
    print("==> Parsing PGPE traces: " + command)
    run_command(command)

    return RC


def parse_32bit(value, stringArray):
    width = 32
    arrayLen = len(stringArray)

    s = ''
    while width:
        if value & 1 == 1:
            if (width-1 < arrayLen):
                if s == "":
                    s = stringArray[width-1]
                else:
                    s = stringArray[width-1] + ", " + s
            else:
                if s == "":
                    s = "bit" + str(width-1)
                else:
                    s = "bit" + str(width-1) + ", " + s
        value //= 2
        width -= 1;

    return s

def bin_header(prefix):
    width = 64
    s_top = ''
    s_bot = ''
    s_sep = ''

    i = 0
    while width > 0:
        s_top += str(i/10)
        s_bot += str(i%10)
        s_sep += "-"
        i += 1
        width -= 1;
        if width % 8 == 0:
            s_top += " "
            s_bot += " "
            s_sep += " "

    print(prefix + s_top)
    print(prefix + s_bot)
    print(prefix + s_sep)


def int_to_bin_string(i):
    width = 64

    #if i == 0:
    #    return "0"

    s = ''
    while i:
        if i & 1 == 1:
            s = "1" + s
        else:
            s = "0" + s
        i //= 2
        width -= 1;
        if (width > 0) and (width % 8) == 0:
            s = " " + s

    while width > 0:
        s = "0" + s
        width -= 1;

    return s


def dump_occ_machine_state(flg_verbose):
    """Dump OCC machine state"""
    RC = 0
    OCC = 0
    ReturnData = []

    if flg_verbose:
        print("Dumping OCC machine state for OCC" + str(OCC))

    print("")
    print("405 Regs:")
    command = "backplane0."+proc+".occ_cmp.proc_405.pregs -all"
    print("==> " + command)
    run_command(command)

    print("GPE0 Regs:")
    command = "backplane0."+proc+".occ_cmp.gpe_ppe0.pregs "
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    print(ReturnData[0])

    print("GPE1 Regs:")
    command = "backplane0."+proc+".occ_cmp.gpe_ppe1.pregs"
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    print(ReturnData[0])

    print("GPE2 (PGPE) Regs:")
    command = "backplane0."+proc+".occ_cmp.gpe_ppe2.pregs"
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    print(ReturnData[0])

    print("Additional Regs:")
    print("")

    # OISR0/OIMR0 - PGPE Interrupts
    command = "backplane0."+proc+".occ_cmp.ocb_base->ocb_oisr0"
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    oisr0 = int(ReturnData[0])
    print("OISR0: " + hex(oisr0))
    bin_header("       ")
    print("OISR0: " + int_to_bin_string(oisr0))
    oisr0Array = [
            "trace_trigger", "occ_error", "gpe2_error", "gpe3_error",
            "check_stop_gpe2", "check_stop_gpe3", "occ_malf_alert", "pvref_error",
            "ipi2", "ipi3", "debug_trigger", "spare",
            "pbax_pgpe_attn", "pbax_pgpe_push0", "pbax_pgpe_push1", "pba_overcurrent_indicator",
            "pmc_pcb_intr_type0_pend", "pmc_pcb_intr_type1_pend", "pmc_pcb_intr_type2_pend", "pmc_pcb_intr_type3_pend",
            "pmc_pcb_intr_type4_pend", "pmc_pcb_intr_type5_pend", "pmc_pcb_intr_type6_pend", "pmc_pcb_intr_type7_pend",
            "pmc_pcb_intr_type8_pend", "pmc_pcb_intr_type9_pend", "pmc_pcb_intr_typeA_pend", "pmc_pcb_intr_typeB_pend",
            "pmc_pcb_intr_typeC_pend", "pmc_pcb_intr_typeD_pend", "pmc_pcb_intr_typeE_pend", "pmc_pcb_intr_typeF_pend",
            ]
    print("       " + parse_32bit(oisr0>>32, oisr0Array))
    command = "backplane0."+proc+".occ_cmp.ocb_base->ocb_oimr0"
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    oimr0 = int(ReturnData[0])
    print("OIMR0: " + hex(oimr0))
    bin_header("       ")
    print("OIMR0: " + int_to_bin_string(oimr0))
    print("       enabled: " + parse_32bit((~oimr0)>>32, oisr0Array))
    merged = oisr0 & (~oimr0)
    print("");
    print("OIS&M0: " + int_to_bin_string(merged))
    print("        " + parse_32bit(merged>>32, oisr0Array))

    # OISR1/OIMR1 - OCC/405 Interrupts
    command = "backplane0."+proc+".occ_cmp.ocb_base->ocb_oisr1"
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    oisr1 = int(ReturnData[0])
    print("OISR1: " + hex(oisr1))
    bin_header("       ")
    print("OISR1: " + int_to_bin_string(oisr1))
    oisr1Array = [
            "debugger", "trace_trigger", "spare", "pba_error",
            "gpe0_error", "gpe1_error", "check_stop_ppc405", "external_trap",
            "occ_timer0", "occ_timer1", "ipi0_hi_priority", "ipi1_hi_priority",
            "ipi4_hi_priority", "i2cm_intr", "ipi_scom", "dcm_intf_ongoing",
            "pbax_occ_send_attn", "pbax_occ_push0", "pbax_occ_push1", "pba_bcde_attn",
            "pba_bcue_attn", "occ_strm0_pull", "occ_strm0_push", "occ_strm1_pull",
            "occ_strm1_push", "occ_strm2_pull", "occ_strm2_push", "occ_strm3_pull",
            "occ_strm3_push", "ipi0_lo_priority", "ipi1_lo_priority", "ipi4_lo_priority"
            ]
    oisr1Array_p9 = [
            "trace_trigger", "occ_error", "gpe2_error", "gpe3_error",
            "check_stop_gpe2", "check_stop_gpe3", "occ_malf_alert", "pvref_error",
            "ipi2", "ipi3", "debug_trigger", "spare",
            "pbax_pgpe_attn", "pbax_pgpe_push0", "pbax_pgpe_push1", "pba_overcurrent_indicator",
            "pmc_pcb_intr_type0_pending", "pmc_pcb_intr_type1_pending", "pmc_pcb_intr_type2_pending", "pmc_pcb_intr_type3_pending",
            "pmc_pcb_intr_type4_pending", "pmc_pcb_intr_type5_pending", "pmc_pcb_intr_type6_pending", "pmc_pcb_intr_type7_pending",
            "pmc_pcb_intr_type8_pending", "pmc_pcb_intr_type9_pending", "pmc_pcb_intr_typea_pending", "pmc_pcb_intr_typeb_pending",
            "pmc_pcb_intr_typec_pending", "pmc_pcb_intr_typed_pending", "pmc_pcb_intr_typee_pending", "pmc_pcb_intr_typef_pending"
            ]
    print("       " + parse_32bit(oisr1>>32, oisr1Array))

    print("")

    command = "backplane0."+proc+".occ_cmp.ocb_base->ocb_oimr1"
    print("==> " + command)
    ReturnData = cli.quiet_run_command(command, output_mode = output_modes.formatted_text)
    oimr1 = int(ReturnData[0])
    print("OIMR1: " + hex(oimr1))
    bin_header("       ")
    print("OIMR1: " + int_to_bin_string(oimr1))
    print("       enabled: " + parse_32bit((~oimr1)>>32, oisr1Array))
    merged = oisr1 & (~oimr1)
    print("");
    print("OIS&M1: " + int_to_bin_string(merged))
    print("        " + parse_32bit(merged>>32, oisr1Array))

    return RC



def sensor_list(flg_verbose):
    """OCC Sensors"""
    RC = 0
    OCC = 0
    ReturnData = []
    sensor_type=0xFFFF
    sensor_loc=0xFFFF
    global G_run_time
    global LastCmd_SeqNum

    print("Dumping OCC sensors for OCC" + str(OCC))


    print("    Sensor Type: 0x" + str('{0:04X}'.format(sensor_type)))
    print("Sensor Location: 0x" + str('{0:04X}'.format(sensor_loc)))

    # Collect the list of applicable sensors (and their sample value)
    more_sensors=1
    first_sensor=0
    while more_sensors:
        # -X 53 0500000001LLLLTTTT = list first 50 OCC sensors (LLLL=Location,TTTT=Type)"
        print("Requesting sensors starting with GUID: 0x"+str('{0:04X}'.format(first_sensor)))
        occ_cmd="53"
        occ_cmd_data="0500"+str('{0:04X}'.format(first_sensor))+"00"+str('{0:04X}'.format(sensor_loc))+str('{0:04X}'.format(sensor_type))
        prior_first_sensor = first_sensor
        #sensor_seq_num=write_cmd_func(flag_Cmd="-C",Cmd_arg=0x53,flag_Data="-D",Data_arg="occ_cmd_data")
        sensor_seq_num=write_cmd_func("-C",0x53,"-D",occ_cmd_data,flg_verbose)

        # let simics run (if not already)
        if G_run_time > 0:
            if L_pgpe_enabled:
                cli.quiet_run_command("run 5 ms", output_mode = output_modes.formatted_text)
            else:
                cli.quiet_run_command("run 2 s", output_mode = output_modes.formatted_text)

        last_sensor = ReadPrint_OCCsensor(FLG_VRB=flg_verbose, SeqNum="00")
        # receive should update more_sensors and last_sensor
        if last_sensor > prior_first_sensor:
            first_sensor = last_sensor + 1
        else:
            more_sensors=0

    print("Done (for now)")


def send_occ_cmd(occ_cmd, occ_data, flg_verbose):
    """Send OCC Command to OCC"""
    RC = 0
    OCC = 0
    ReturnData = []
    retries = 2
    global G_run_time
    global LastCmd_SeqNum
    timeout = 1

    if L_pgpe_enabled == 1:
        retries = 10
        if occ_cmd == 0x20:
            # SET STATE OR MODE takes longer
            timeout = 50
        else:
            timeout = 5

    print("Sending 0x"+"{:02x}".format(occ_cmd)+" command to OCC");

    seq_num=write_cmd_func("-C", occ_cmd, "-D", occ_data, flg_verbose)
    print("Seq="+str(seq_num))
    seq_num=0 # Ignore seq checking

    while retries > 0:
        if G_run_time > 0:
            # let simics run (if not already)
            if L_pgpe_enabled:
                cli.quiet_run_command("run "+str(timeout)+" ms", output_mode = output_modes.formatted_text)
            else:
                cli.quiet_run_command("run "+str(timeout)+" s", output_mode = output_modes.formatted_text)

        RC = rValidate_cmd_func("-C", occ_cmd, "-D", LastCmd_SeqNum, flg_verbose, "", "")
        if RC != 0:
            if RC == 0xFD:
                retries = retries - 1
            else:
                print("ERROR: returning "+str(RC))
                retries = 0
        else:
            retries = 0
        if retries > 0:
            print("TIMEOUT: Re-checking RSP buffer (retries="+str(retries)+")")

    if RC != 0:
        print("ERROR: returning "+str(RC))

    return RC


#===============================================================================
#
# The following define new commands to be used by simics
#
#===============================================================================

#===============================================================================
#
# Write a command to the OCC command buffer (send command to the OCC)
#
#===============================================================================
new_command("OCCwrite",             # The name of the command - required
            write_cmd_func,         # The function to call when the command is
                                    # issued - required
            args = [                # List of arguments to the command - required
                    arg(flag_t, "-C" ),
                    arg(int_t,  "CMD_arg", "1", None),
                    arg(flag_t, "-D" ),
                    arg(str_t,  "DATA_arg", "1", None),
                    arg(flag_t, "-v" ),
                   ],
        alias = "OCCw",             # Alias for the command name
        short = "An example command", # Short command description
        type = ["ibm-util"],        # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>OCCwrite</b> -C 0x<CMD> -D <DATA> [-v]<br>

<b>-C</b> &nbsp;
<br>&nbsp;&nbsp;- Command Flag<br>

<b>CMD_arg</b> &nbsp; string
<br>&nbsp;&nbsp;- a required Command argument<br>

<b>-D</b> &nbsp;
<br>&nbsp;&nbsp;- Data Flag<br>

<b>DATA_arg</b> &nbsp; string
<br>&nbsp;&nbsp;- a required Data Commnad argument<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)



#===============================================================================
#
# Read a response from the OCC response buffer (read response from the OCC)
#
#===============================================================================
new_command("OCCread",              # The name of the command - required
            read_cmd_func,          # The function to call when the command is
                                    # issued - required
            args = [                # List of arguments to the command - required
                    arg(flag_t, "-C" ),
                    arg(int_t,  "CMD_arg", "1", None),
                    arg(flag_t, "-v" ),
                   ],
        alias = "OCCr",             # Alias for the command name
        short = "An example command", # Short command description
        type = ["ibm-util"],        # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>OCCread</b> -C 0x<CMD> [-v]<br>

<b>-C</b> &nbsp;
<br>&nbsp;&nbsp;- Command Flag<br>

<b>CMD_arg</b> &nbsp; string
<br>&nbsp;&nbsp;- a required Command argument<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)

#===============================================================================
#
# Read a response from the OCC response buffer (read response from the OCC)
# and validate the response
#
#===============================================================================
new_command("OCCreadvalidate",      # The name of the command - required
            rValidate_cmd_func,     # The function to call when the command is
                                    # issued - required
            args = [                # List of arguments to the command - required
                    arg(flag_t, "-C" ),
                    arg(int_t,  "CMD_arg", "1", None),
                    arg(flag_t, "-D" ),
                    arg(int_t,  "DATA_arg", "1", None),
                    arg(flag_t, "-v" ),
                    arg(flag_t, "-L" ),
                    arg(flag_t, "-q" ),
                   ],
        alias = "OCCrv",            # Alias for the command name
        short = "An example command", # Short command description
        type = ["ibm-util"],        # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>OCCreadvalidate</b> -C 0x<CMD> -D 0x<seq> [-v]<br>

<b>-C</b> &nbsp;
<br>&nbsp;&nbsp;- Command Flag<br>

<b>CMD_arg</b> &nbsp; string
<br>&nbsp;&nbsp;- a required Command argument<br>

<b>-D</b> &nbsp;
<br>&nbsp;&nbsp;- Data Flag<br>

<b>DATA_arg</b> &nbsp; string
<br>&nbsp;&nbsp;- a required Data Commnad argument<br>

<b>-L</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- last read attempt (trace buffer on timeout)<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

<b>-q</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- minimal output on fail (quiet)<br>


"""
)

#===============================================================================
#
# Dump and parse the OCC traces using occStringFile from specified in directory
#
#===============================================================================
new_command("OCCtrace",          # The name of the command - required
            occ_trace,           # The function to call when the command is issued - required
            args = [             # List of arguments to the command - required
                    arg(str_t,  "string", "?", None),
                    arg(flag_t, "-v" ),
                    arg(flag_t, "-D" ),
                    arg(str_t,  "TARGET_DIR", "?", None), # Optional Parameter
                    arg(flag_t, "-P" ),
                    arg(str_t,  "PREFIX_arg", "?", None), # Optional Parameter
                   ],
        alias = "occt",           # Alias for the command name
        short = "Dump OCC trace", # Short command description
        type = ["ibm-util"],      # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>OCCtrace</b> string [-v] [-P TRACEPREFIX] - Display OCC trace<br>

<b>string</b> &nbsp; [optional] one of<br>
         Directory name containing occStringFile, or <br>
         Filename of OCC string file, or<br>
         Github base directory containing OCC code<br>

<b>-D</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- Save traces to specified directory: TARGET_DIR<br>
&nbsp;&nbsp;  If specified, then "string" must also be specified<br>

<b>-P</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- Save trace with specified prefix (TRACEPREFIX_occMERG.txt)<br>
&nbsp;&nbsp;  If specified, then -D must also be specified<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)

#===============================================================================
#
# Dump and parse the OCC traces using trexStringFile from specified in directory
#
#===============================================================================
new_command("PGPEtrace",         # The name of the command - required
            pgpe_trace,          # The function to call when the command is issued - required
            args = [             # List of arguments to the command - required
                    arg(str_t,  "string", "?", None),
                    arg(flag_t, "-v" ),
                   ],
        alias = "pgpet",          # Alias for the command name
        short = "Dump PGPE trace", # Short command description
        type = ["ibm-util"],      # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>PGPEtrace</b> string [-v] - Display PGPE trace<br>

<b>string</b> &nbsp; [optional] one of<br>
         Directory name containing PGPE trexStringFile, or <br>
         Filename of PGPE string file<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)

#===============================================================================
#
# Dump the OCC machine state information (registers, etc)
#
#===============================================================================
new_command("OCCstate",         # The name of the command - required
            dump_occ_machine_state,           # The function to call when the command is issued - required
            args = [            # List of arguments to the command - required
                    arg(flag_t, "-v" ),
                   ],
        alias = "occstate",     # Alias for the command name
        short = "Dump OCC Machine State", # Short command description
        type = ["ibm-util"],    # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>OCCstate</b> [-v] - Display OCC machine state<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)

#===============================================================================
#
# Dump the OCC sensor list
#
#===============================================================================
new_command("OCCsensorlist",    # The name of the command - required
            sensor_list,        # The function to call when the command is issued - required
            args = [            # List of arguments to the command - required
                    arg(flag_t, "-v" ),
                   ],
        alias = "sensorlist",   # Alias for the command name
        short = "Dump OCC Sensor List", # Short command description
        type = ["ibm-util"],    # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>OCCstate</b> [-v] - Display OCC sensor list<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)

#===============================================================================
#
# Send specified config data to the OCC
#
#===============================================================================
new_command("occcmd",       # The name of the command - required
            send_occ_cmd,       # The function to call when the command is issued - required
            args = [            # List of arguments to the command - required
                    arg(int_t,  "occ_command"),
                    arg(str_t,  "command_data_string"),
                    arg(flag_t, "-v" ),
                   ],
        alias = "ocmd",     # Alias for the command name
        short = "Send OCC Command", # Short command description
        type = ["ibm-util"],    # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>occcmd</b> occ_cmd cmd_data_string [options] - Send command to OCC<br>

<b>occ_cmd</b> OCC command to send (integer)<br>
<b>cmd_data_string</b> Command data to send (hexstring)<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)


#===============================================================================
#
# The following are predefined functions to perform OCC testing
#
#===============================================================================

#===============================================================================
# Prep simics to run OCC code
#===============================================================================
def occ_init(code_dir, flg_pgpe, flg_verbose):
    """Initialization to get OCC ready to run"""
    RC = 0
    OCC = 0
    ReturnData = []

    # Required for p10_standalone
    command = "system_cmp0.cpu0_0_00_0.disable"
    print("==> " + command);
    cli.run_command(command)

    # Hide error message: [backplane0.dcm0.chip0.pib_cmp.tod_scom error] Mambo cpu system_cmp0.cpu0_0_00_0 is disabled - Failed reading chip TOD value
    command = "backplane0."+proc+".pib_cmp.tod_scom.log-type -disable log-type = error"
    print("==> " + command);
    cli.run_command(command)

    command = "backplane0->system_info = \"OCC Complex Model\""
    print("==> " + command)
    cli.run_command(command)

    base_dir = "binaries"
    if code_dir:
        base_dir = code_dir
    occ_405_binary_to_load  = base_dir + "/occ_405.out"
    occ_gpe0_binary_to_load = base_dir + "/occ_gpe0.out"
    occ_gpe1_binary_to_load = base_dir + "/occ_gpe1.out"
    occ_gpe2_binary_to_load = base_dir + "/pgpe.out"

    # Load the code binaries into OCI space
    print("\n==> Loading 405 binary: " + occ_405_binary_to_load)
    cli.run_command("!ls -l "+occ_405_binary_to_load)
    command = "backplane0."+proc+".occ_cmp.oci_space.load-binary -v "+occ_405_binary_to_load
    print("==> " + command)
    cli.run_command(command)
    command = "new-symtable -n sym_tbl_405 file="+occ_405_binary_to_load
    print("==> " + command)
    cli.run_command(command)
    command = "new-context ctx_405"
    print("==> " + command)
    cli.run_command(command)
    command = "backplane0."+proc+".occ_cmp.proc_405.set-context context = ctx_405"
    print("==> " + command)
    cli.run_command(command)
    command = "ctx_405->symtable = sym_tbl_405"
    print("==> " + command)
    cli.run_command(command)

    print("\n==> Loading GPE0 binary: " + occ_gpe0_binary_to_load)
    cli.run_command("!ls -l "+occ_gpe0_binary_to_load)
    command = "backplane0."+proc+".occ_cmp.oci_space.load-binary -v "+occ_gpe0_binary_to_load
    print("==> " + command)
    cli.run_command(command)
    command = "new-symtable -n sym_tbl_gpe0 file="+occ_gpe0_binary_to_load
    print("==> " + command)
    cli.run_command(command)
    command = "backplane0."+proc+".occ_cmp.gpe_ctx0.symtable sym_tbl_gpe0"
    print("==> " + command)
    cli.run_command(command)

    print("\n==> Loading GPE1 binary: " + occ_gpe1_binary_to_load)
    cli.run_command("!ls -l "+occ_gpe1_binary_to_load)
    command = "backplane0."+proc+".occ_cmp.oci_space.load-binary -v "+occ_gpe1_binary_to_load
    print("==> " + command)
    cli.run_command(command)
    command = "new-symtable -n sym_tbl_gpe1 file="+occ_gpe1_binary_to_load
    print("==> " + command)
    cli.run_command(command)
    command = "backplane0."+proc+".occ_cmp.gpe_ctx1.symtable sym_tbl_gpe1"
    print("==> " + command)
    cli.run_command(command)

    if flg_pgpe:
        print("\n==> Loading GPE2 (PGPE) binary: " + occ_gpe2_binary_to_load)
        cli.run_command("!ls -l "+occ_gpe2_binary_to_load)
        command = "backplane0."+proc+".occ_cmp.oci_space.load-binary -v "+occ_gpe2_binary_to_load
        print("==> " + command)
        cli.run_command(command)
        command = "new-symtable -n sym_tbl_gpe2 file="+occ_gpe2_binary_to_load
        print("==> " + command)
        cli.run_command(command)
        command = "backplane0."+proc+".occ_cmp.gpe_ctx2.symtable sym_tbl_gpe2"
        print("==> " + command)
        cli.run_command(command)

        #command = "backplane0."+proc+".occ_cmp.oci_space.write 0xfff201a0 0x10300000 -b" # Enable PGPE immediate return
        #print("\n==> " + command + " (enable PGPE immediate return)")
        #cli.run_command(command)

    #print("\n==> Enable GPE2 (needed?)")
    #cli.run_command("backplane0."+proc+".occ_cmp.gpe_ppe2.enable") #Enable GPE2

    # GPE Interrupt Vector Prefix Register
    print("\n==> Initialize IVPR and PIR")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0000008 0xFFF01000 -b") # TP.TPCHIP.OCC.OCI.GPE0.GPEIVPR
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0010008 0xFFF10000 -b") # TP.TPCHIP.OCC.OCI.GPE1.GPEIVPR
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0020008 0xFFF20000 -b") # TP.TPCHIP.OCC.OCI.GPE2.GPEIVPR
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0030008 0xFFF30000 -b") # TP.TPCHIP.OCC.OCI.GPE3.GPEIVPR

    cli.run_command("backplane0."+proc+".occ_cmp.gpe_ppe0.write-reg pir value=0x00000000")
    cli.run_command("backplane0."+proc+".occ_cmp.gpe_ppe1.write-reg pir value=0x00000001")
    cli.run_command("backplane0."+proc+".occ_cmp.gpe_ppe2.write-reg pir value=0x00000002")
    cli.run_command("backplane0."+proc+".occ_cmp.gpe_ppe3.write-reg pir value=0x00000003")

    # PPE External Interface XCR - Start each GPE? - Hard Reset, Toggle XSR[TRH], Resume
    print("\n==> Start GPE0 (Hard Reset, Toggle XSR, Resume)")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0000088 0x6000000000000000 8 -b") # GPE0
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0000088 0x4000000000000000 8 -b")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0000088 0x2000000000000000 8 -b")

    print("\n==> Start GPE1 (Hard Reset, Toggle XSR, Resume)")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0010088 0x6000000000000000 8 -b") # GPE1
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0010088 0x4000000000000000 8 -b")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0010088 0x2000000000000000 8 -b")

    if flg_pgpe:
        print("\n==> Start GPE2 (Hard Reset, Toggle XSR, Resume)")
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0020088 0x6000000000000000 8 -b") # GPE2
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0020088 0x4000000000000000 8 -b")
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xc0020088 0x2000000000000000 8 -b")

    ### SRAM LAYOUT (768KB) ###
    # FFF00000 - IPC Communication Region
    # FFF01000 - GPE0 Code (60K)
    # FFF10000 - GPE1 Code (64K)
    # FFF20000 - PGPE HCode+Data (48K)
    # FFF2C000 - SGPE HCode (80K)
    # FFF40000 - 405 Code+Data (512K)
    # FFFB3C00 -     ERR Trace Buffer (8K)
    # FFFB5C00 -     INF Trace Buffer (8K)
    # FFFB7C00 -     IMP Trace Buffer (8K)
    # FFFB9C00 -     FIR Heap / Parms (16K)
    # FFFBDC00 -     FSP/BMC Response Buffer (4K)
    # FFFBEC00 -     FSP/BMC Command Buffer (4K)
    # FFFBDC00 -     Reserved (Bootloader) (1K)

    # HOMER Layout (relative to base address 0x80000000)
    # 000C0000 - header? version (4 bytes), nestFreq (4B), interruptType (4B), firMaster (4B), firParms, smfMode (4B)
    # 000E0000 - HTMGT Command Buffer (4k)
    # 000E1000 - HTMGT Response Buffer (4k)
    # 000E2000 - OPAL Shared Memory (32k)
    # 000E2000 -   OPAL Static space (0x880 bytes)
    # 000E2B80 -   OPAL Dynamic space (128 bytes)
    # 000E2C00 -   Inband Command Buffer (4k)
    # 000E3C00 -   Inband Response Buffer (4k)
    # 00300000 - PPMR Header (4k)

    # Request a 405 chip level reset and set the 405 pc
    cli.run_command("backplane0."+proc+".occ_cmp.proc_405.write-reg reg-name = dbcr0 value = 0x20000000")
    if G_bypass_bootloader == 1:
        # Set PC to 405 code (bypass OCC bootloader)
        print("==> Requesting 405 chip level reset and set pc to __ssx_boot (OCC 405 code - no bootloader)");
        cli.run_command("backplane0."+proc+".occ_cmp.proc_405.write-reg reg-name = pc value = (backplane0."+proc+".occ_cmp.proc_405.sym __ssx_boot)")
    else:
        # Set PC to default (will start with OCC bootloader)
        print("==> Requesting 405 chip level reset and set pc to 0xFFFFFFFC (OCC bootloader will run)");
        cli.run_command("backplane0."+proc+".occ_cmp.proc_405.write-reg reg-name = pc value = 0xFFFFFFFC") # Start boot loader

        print("==> Write instruction to branch to start of 405 code (0xFFF40000) at SRAM Boot Vector 3 (0xFFFFFFFC)");
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xFFFFFFFC 0x4BF40002 size = 4")

        # SRAM - FFF4000 is start of PPC405 Code
        print("==> Write instructions to branch to bootloader (start of HOMER) at start of 405 code (0xFFF40000)");
        # (branch is too far for single instruction, so must use CTR)
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xFFF40000 0x3C208000 size = 4") # lis     r1,-32768
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xFFF40004 0x60210040 size = 4") # ori     r1,r1,64
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xFFF40008 0x7C2903A6 size = 4") # mtctr   r1
        cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0xFFF4000C 0x4E800420 size = 4") # bctr

    # Create 16K OCC cache
    print("==> Creating 16K OCC cache");
    cli.run_command("@will_cache = SIM_create_object(\"image\", \"occ_cache\", [[\"size\",0x4000]])")
    cli.run_command("backplane0."+proc+".occ_cmp.ram_cacheable->image = occ_cache")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.del-map base = 0x00000000 device = backplane0."+proc+".occ_cmp.ram_cacheable")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.add-map base = 0x00000000 device = backplane0."+proc+".occ_cmp.ram_cacheable length = 0x4000 align-size = 8192")

    # Load the code binaries into main memory
    if G_bypass_bootloader != 1:
        print("==> Loading OCC image: ${simics_binaries}/image.bin @ 0x00000000")
        cli.run_command("system_cmp0.memory_image0.load-file binaries/image.bin 0x00000000")
    print("==> Loading fake PPMR header (ppmr.bin)")
    cli.run_command("system_cmp0.memory_image0.load-file binaries/ppmr.bin  0x00300000")
    print("==> Loading fake OPPB header (oppb.bin)")
    cli.run_command("system_cmp0.memory_image0.load-file binaries/oppb.bin  0x00320000")

    # Dump 405 PC
    cli.run_command("backplane0."+proc+".occ_cmp.proc_405.read-reg reg-name = pc")

    print("==> Enabling 405");
    cli.run_command("backplane0."+proc+".occ_cmp.proc_405.enable")

    # HOMER init
    print("==> Initialize OCC Host Config Data in HOMER (0x800F4000)")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0x800F4000 0x000000A0 -b") # Version
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0x800F4004 0x00000964 -b") # Nest Frequency
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0x800F4008 0x00000000 -b") # Interrupt Type (FSP)
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.write 0x800F400C 0x00000000 -b") # SMB
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.read  0x800F4000 8")
    cli.run_command("backplane0."+proc+".occ_cmp.oci_space.read  0x800F4008 8")

    print("==> Write PBABARMSK0 to 0x7F00000");
    cli.run_command("backplane0."+proc+".bridge_cmp.pba->pba_barmsk0=0x0000000007f00000")

    print("==> INIT APSS CHANNEL VALUES")
    command = "backplane0.apss->adc_channel_val = [0x0293, 0x9287, 0x2887, 0x3877, 0x4326, 0x52F2, 0x6214, 0x7006, 0x8003, 0x9EB7, 0xA01B, 0xB013, 0xC123, 0xD0FC, 0xE0F2, 0xF008]"
    print("==> " + command)
    cli.run_command(command)

    cli.run_command("backplane0."+proc+".occ_cmp.proc_405.read-reg reg-name = pc")

    command = "backplane0."+proc+".occ_cmp.ocb_agen->OCB_AGEN_CCSR" # Display CCSR
    print("==> " + command)
    cli.run_command(command)

    if flg_pgpe:
        print("==> ENABLING 32 CORES...")
        command = "backplane0."+proc+".occ_cmp.ocb_agen->OCB_AGEN_CCSR = 0xffffff0000000000" # Set CCSR
        print("==> " + command)
        cli.run_command(command)
        command = "backplane0."+proc+".occ_cmp.ocb_agen->OCB_AGEN_CCSR" # Display CCSR
        print("==> " + command)
        cli.run_command(command)

    if (G_bypass_bootloader != 1) and flg_pgpe:
        # Allow bootloader to run a while (sram test/copy code images)
        cli.quiet_run_command("run 40 ms", output_mode = output_modes.formatted_text)

    return RC


def wait_for_checkpoint(flg_verbose):
    keep_waiting = True
    while keep_waiting:
        sramRC, mylist = ReadSRAM("backplane0."+proc+".occ_cmp.oci_space.x "+G_occsram_rsp_buffer+" 16 " , flg_verbose)
        SeqNum = int( mylist[0][:2], 16 )
        Cmd = int( mylist[0][-2:], 16 )
        RtnCode = int(mylist[1][:2],16); # Convert to integer
        DataLength = int(mylist[1][-2:] + mylist[2][:2],16)
        CheckPoint = int(mylist[3], 16)
        if (SeqNum != 0) or (Cmd != 0):
            print("WARNING: Unexpected sequence # ("+str(SeqNum)+") or command ("+str(Cmd)+") - communication w/OCC already started?");
            keep_waiting = False
        if RtnCode == 0xE1:
            if (CheckPoint == 0x0EFF):
                print("occ_to_active: Initialization Checkpoint 0x%04X (COMPLETE)" % CheckPoint + " (data length="+str(DataLength)+")")
                keep_waiting = False
            else:
                print("occ_to_active: Initialization Checkpoint 0x%04X" % CheckPoint + " (data length="+str(DataLength)+")")
        elif RtnCode != 0x00:
            print("ERROR: UNEXPECTED STATUS ("+str(RtnCode)+")");
            keep_waiting = False
        if keep_waiting:
            if G_run_time > 0:
                # let simics run (if not already)
                if L_pgpe_enabled:
                    cli.quiet_run_command("run 5 ms", output_mode = output_modes.formatted_text)
                else:
                    cli.quiet_run_command("run 1 s", output_mode = output_modes.formatted_text)


#===============================================================================
# Wait for OCCs to complete init and then send one poll command
#===============================================================================
def occ_to_ckpt(flg_pgpe, flg_run, flg_verbose):
    """Wait for checkpoint and send Poll command"""
    RC = 0
    OCC = 0
    ReturnData = []
    global G_run_time
    global LastCmd_SeqNum
    global L_pgpe_enabled

    if flg_pgpe:
        L_pgpe_enabled = 1

    print("#### WAIT FOR OCC CHECKPOINT #############################################################################");
    if flg_run:
        # Simics is already running (do not issue any manual run commands)
        G_run_time = 0
    wait_for_checkpoint(flg_verbose);

    print("#### SEND POLL COMMAND ###################################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    return RC

#===============================================================================
# Move OCCs to Active state (must be run after occ_init)
#===============================================================================
def occ_to_active(flg_pgpe, flg_run, flg_verbose):
    """Config Required for Active State"""
    RC = 0
    OCC = 0
    ReturnData = []
    global G_run_time
    global LastCmd_SeqNum
    global L_pgpe_enabled

    if flg_pgpe:
        L_pgpe_enabled = 1

    print("#### WAIT FOR OCC CHECKPOINT #############################################################################");
    if flg_run:
        # Simics is already running (do not issue any manual run commands)
        G_run_time = 0
    wait_for_checkpoint(flg_verbose);

    print("#### SEND POLL COMMAND ###################################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    print("\n#### SET SYSTEM CONFIG ###################################################################################");
    cmdData="0F3001" # single node
    cmdData=cmdData+"00000001" # proc sensor id
    cmdData=cmdData+"00000002" # proc frequency sensor id
    cmdData=cmdData+"01000001" # core0 temp sensor id
    cmdData=cmdData+"01000101"
    cmdData=cmdData+"01000201"
    cmdData=cmdData+"01000301"
    cmdData=cmdData+"01000401"
    cmdData=cmdData+"01000501"
    cmdData=cmdData+"01000601"
    cmdData=cmdData+"01000701"
    cmdData=cmdData+"01000801"
    cmdData=cmdData+"01000901"
    cmdData=cmdData+"01001001"
    cmdData=cmdData+"01001101"
    cmdData=cmdData+"01001201"
    cmdData=cmdData+"01001301"
    cmdData=cmdData+"01001401"
    cmdData=cmdData+"01001501"
    cmdData=cmdData+"01001601"
    cmdData=cmdData+"01001701"
    cmdData=cmdData+"01001801"
    cmdData=cmdData+"01001901"
    cmdData=cmdData+"01002001"
    cmdData=cmdData+"01002101"
    cmdData=cmdData+"01002201"
    cmdData=cmdData+"01002301"
    cmdData=cmdData+"01002401"
    cmdData=cmdData+"01002501"
    cmdData=cmdData+"01002601"
    cmdData=cmdData+"01002701"
    cmdData=cmdData+"01002801"
    cmdData=cmdData+"01002901"
    cmdData=cmdData+"01003001"
    cmdData=cmdData+"01003101"
    cmdData=cmdData+"02000001" # backplane sensor
    cmdData=cmdData+"03000001" # APSS sensor
    cmdData=cmdData+"04000001" # VRM VDD Sensor ID
    cmdData=cmdData+"05000001" # VRM VDD Temp Sensor ID
    cmdData=cmdData+"0000000000000000" # Reserved
    RC = send_occ_cmd(0x21, cmdData, flg_verbose);

    print("\n#### SET APSS CONFIG #####################################################################################");
    cmdData="04200000"
    cmdData=cmdData+"01AAAAAA010000004E4800000005" # channel 0 - Mem Proc 0
    cmdData=cmdData+"02AAAAAA020000004E4800000005" # channel 1 - Mem Proc 1
    cmdData=cmdData+"05AAAAAA030000004E4800000005" # channel 2 - Proc 0
    cmdData=cmdData+"06AAAAAA040000004E4800000005" # channel 3 - Proc 1
    cmdData=cmdData+"09AAAAAA05000000271000000005" # channel 4 - Proc 0 cache/io/pcie
    cmdData=cmdData+"0AAAAAAA06000000271000000005" # channel 5 - Proc 1 cache/io/pcie
    cmdData=cmdData+"12AAAAAA07000000239D00000005" # channel 6 - Storage A
    cmdData=cmdData+"13AAAAAA08000000239D00000005" # channel 7 - Storage B
    cmdData=cmdData+"15AAAAAA0900000003E800000000" # channel 8 - ground
    cmdData=cmdData+"14AAAAAA0A000000195E00000005" # channel 9 - 12v sense
    cmdData=cmdData+"0DAAAAAA0B000000239D00000005" # channel 10 - IO A
    cmdData=cmdData+"0EAAAAAA0C000000239D00000005" # channel 11 - IO B
    cmdData=cmdData+"0FAAAAAA0D000000239D00000005" # channel 12 - IO C
    cmdData=cmdData+"10AAAAAA0E000000239D00000005" # channel 13 - Fans A
    cmdData=cmdData+"16AAAAAA0F000000239D00000005" # channel 14 - Total System Power : DEBUG: verify this is used to report total power (in CAPS in poll rsp)
    cmdData=cmdData+"18AAAAAA100000004E4800000005" # channel 15 - GPU
    cmdData=cmdData+"00000000000005060708" # GPIO port 0
    cmdData=cmdData+"0300090A000000000000" # GPIO port 1
    RC = send_occ_cmd(0x21, cmdData, flg_verbose);


    print("\n#### POLL TO VERIFY NO ELOGS #############################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    print("\n#### SET ROLE COMMAND - MASTER ###########################################################################");
    RC = send_occ_cmd(0x21, "0301000", flg_verbose);

    print("\n#### POLL TO VERIFY NO ELOGS #############################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    print("\n#### SET MEMORY CONFIG ###################################################################################");
    RC = send_occ_cmd(0x21, "0521000000", flg_verbose);

    print("\n#### SET THERMAL THRESHOLDS (TCT) ########################################################################");
    RC = send_occ_cmd(0x21, "1330140A080200FF5FFFFFFF02FF48FFFF1E", flg_verbose);

    print("\n#### SET AVSBUS CONFIG ###################################################################################");
    RC = send_occ_cmd(0x21, "1430FF0000000000", flg_verbose);

    print("\n#### SET PCAP DATA #######################################################################################");
    RC = send_occ_cmd(0x21, "0720011F012C07E907C2", flg_verbose);

    print("\n#### SET MEMORY THROTTLE #################################################################################");
    cmdData="123004"
    #                            WOF-BASE    PCAP        FMAX        reserved
    cmdData=cmdData+"000044440100233334440120233334440110233334440130EEEEEEEEEEEE"
    cmdData=cmdData+"010044440101455556660121233334440111677778880131EEEEEEEEEEEE"
    cmdData=cmdData+"020044440102455556660122233334440112677778880132EEEEEEEEEEEE"
    cmdData=cmdData+"030044440103455556660123233334440113677778880133EEEEEEEEEEEE"
    RC = send_occ_cmd(0x21, cmdData, flg_verbose);

    print("\n#### SET GPU CONFIG ######################################################################################");
    RC = send_occ_cmd(0x21, "15020000000000010000", flg_verbose);

    print("\n#### VALIDATE POLL DATA ##################################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    print("\n#### SET STATE - ACTIVE ##################################################################################");
    RC = send_occ_cmd(0x20, "300300000000", flg_verbose);

    print("\n#### VALIDATE POLL DATA ##################################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    print("\n#### SET NOMINAL MODE ####################################################################################");
    RC = send_occ_cmd(0x20, "300001000000", flg_verbose);

    print("\n#### VALIDATE POLL DATA ##################################################################################");
    RC = send_occ_cmd(0x00, "20", flg_verbose);

    print("\nDone (for now)")

    return RC


#===============================================================================
#
# The following define new commands for OCC Testing (from p10_OCC_Tests.py)
#
#===============================================================================

#===============================================================================
#
# Prep simics to run OCC code
#
#===============================================================================
new_command("occinit",          # The name of the command - required
            occ_init,           # The function to call when the command is issued - required
            args = [            # List of arguments to the command - required
                    arg(str_t,  "dir", "?", None),
                    arg(flag_t, "-p" ),
                    arg(flag_t, "-v" ),
                   ],
        alias = "oinit",     # Alias for the command name
        short = "Prep simics to run OCC code", # Short command description
        type = ["ibm-util"],    # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>occinit</b> [dir] [-p] [-v] - Prep simics to run OCC code<br>

<b>dir</b> &nbsp; Directory name containing code images<br>

<b>-p</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- also enable PGPE<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)

#===============================================================================
#
# Send required config data to get to active state
#
#===============================================================================
new_command("occ2ckpt",         # The name of the command - required
            occ_to_ckpt,        # The function to call when the command is issued - required
            args = [            # List of arguments to the command - required
                    arg(flag_t, "-p" ),
                    arg(flag_t, "-r" ),
                    arg(flag_t, "-v" ),
                   ],
        alias = "o2c",     # Alias for the command name
        short = "Wait for OCC init complete checkpoint and send poll", # Short command description
        type = ["ibm-util"],    # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>occ2ckpt</b> [-r] [-v] - Wait for OCC init complete checkpoing and send poll<br>

<b>-p</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- also enable PGPE<br>

<b>-r</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- simics is already running (do not run between commands)<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)


#===============================================================================
#
# Send required config data to get to active state
#
#===============================================================================
new_command("occ2act",          # The name of the command - required
            occ_to_active,      # The function to call when the command is issued - required
            args = [            # List of arguments to the command - required
                    arg(flag_t, "-p" ),
                    arg(flag_t, "-r" ),
                    arg(flag_t, "-v" ),
                   ],
        alias = "o2a",     # Alias for the command name
        short = "Send Config required for Obs state", # Short command description
        type = ["ibm-util"],    # Command categories
        # Command documentation, supports simple HTML like markup
        doc =
"""
<b>occ2act</b> [-r] [-v] - Send config required to reach Active state<br>

<b>-p</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- also enable PGPE<br>

<b>-r</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- simics is already running (do not run between commands)<br>

<b>-v</b> &nbsp; [optional]
<br>&nbsp;&nbsp;- verbose mode flag<br>

"""
)
