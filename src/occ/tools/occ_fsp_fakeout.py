# @file - occ_fsp_fakeout.py
# @brief Create Simics Commands for Reading AMEC Sensors
# 
#   Flag    Def/Fea    Userid    Date        Description
#   ------- ---------- --------  ----------  ----------------------------------
#                      thallet   07/01/2012  Created


from time import sleep
from random import randint
import sys
import re

import struct
import array
import os

G_occ_command = []
G_occ_response = []
G_occ_resp_rc = {0x00:'ERRL_RC_SUCCESS',
        0x01:'ERRL_RC_CONDITIONAL_SUCCESS',
        0x11:'ERRL_RC_INVALID_CMD',
        0x12:'ERRL_RC_INVALID_CMD_LEN',
        0x13:'ERRL_RC_INVALID_DATA',
        0x14:'ERRL_RC_CHECKSUM_FAIL',
        0x15:'ERRL_RC_INTERNAL_FAIL',
        0x16:'ERRL_RC_INVALID_STATE'}

def occ_calc_cksm( ):
    global G_occ_command
    cksm = sum( G_occ_command )
    cksm_16 = cksm & 0xFFFF
    cksm_hi = cksm_16/256
    cksm_lo = cksm_16%256
    cksm_data = [cksm_hi,cksm_lo]
    return cksm_data


def occ_send_command( occ_idx ):
    simics_occ_send_commands = []

    # SCOM to set up OCBAR
    simics_occ_send_commands.append("p8Proc" + occ_idx + ".proc_chip.invoke parallel_store SCOM 0x6b010 \"0xffff6000_00000000\" 64")

    # SCOMs to send data to OCC sram (8bytes per scom, so must pad)
    full = (len(G_occ_command)/8)
    partial = (len(G_occ_command)%8)
    
    # Pad out to 8byte boundry
    for byte in range((8-partial)):
        G_occ_command.append(0);

    # Build up simics commands
    for byte in range(full+1):
        simics_occ_send_commands.append( "p8Proc" + occ_idx + ".proc_chip.invoke parallel_store SCOM 0x6b015 \"0x" + \
        '%02x' % (G_occ_command[((byte*8) + 0)]) + \
        '%02x' % (G_occ_command[((byte*8) + 1)]) + \
        '%02x' % (G_occ_command[((byte*8) + 2)]) + \
        '%02x' % (G_occ_command[((byte*8) + 3)]) + \
        "_" + \
        '%02x' % (G_occ_command[((byte*8) + 4)]) + \
        '%02x' % (G_occ_command[((byte*8) + 5)]) + \
        '%02x' % (G_occ_command[((byte*8) + 6)]) + \
        '%02x' % (G_occ_command[((byte*8) + 7)]) + \
        "\" 64")

    # SCOM to send doorbell to OCC
    simics_occ_send_commands.append("p8Proc" + occ_idx + ".proc_chip.invoke parallel_store SCOM 0x6b035 \"0x11111111_22222222\" 64")
    
    # Send SCOMs
    for line in  simics_occ_send_commands:
        cli.quiet_run_command(line)


def occ_read_command( occ_idx, i_length=128 ):
    simics_occ_read_commands = []
    
    # SCOM to set up OCBAR
    simics_occ_read_commands.append("p8Proc" + occ_idx + ".proc_chip.invoke parallel_store SCOM 0x6b010 \"0xffff7000_00000000\" 64")

    # Build up simics commands to read the requested amount of data from SRAM
    for doublewords in range((i_length/8)):
        simics_occ_read_commands.append("p8Proc" + occ_idx + ".proc_chip.invoke parallel_load SCOM 0x6b015 64")

    # Send SCOMs
    for line in  simics_occ_read_commands:
        rsp = cli.run_command(line)


def occ_read_command_sram_direct( occ_idx ):
    global G_occ_response

    # Clear out response buffer
    G_occ_response = []

    simics_occ_read_commands = []
    
    # Build up simics command to read SRAM
    simics_occ_read_commands.append("p8Proc" + occ_idx + ".OccComplexSlot.OccSimpleSlot.cpu.x 0xffff7000 128")

    # Convert SRAM read into bytes, then fill in G_occ_response array
    for line in  simics_occ_read_commands:
        data = []
        rsp = cli.quiet_run_command(line)

        # Loop through lines returned, and do patter matching to split into bytes
        for byteline in rsp[1].split('\n'):
           #Pattern Example:  v:0xffff7000  aa00 0000 0b03 0101 ff00 0000 00f4 0506  ................
           data = re.match(r'(v.0xffff7...)  (..)(..) (..)(..) (..)(..) (..)(..) (..)(..) (..)(..) (..)(..) (..)(..)  (.{16})',byteline,re.M)
           # If match, add to G_occ_response
           if data:
               G_occ_response.append(int(data.group(2) ,16))
               G_occ_response.append(int(data.group(3) ,16))
               G_occ_response.append(int(data.group(4) ,16))
               G_occ_response.append(int(data.group(5) ,16))
               G_occ_response.append(int(data.group(6) ,16))
               G_occ_response.append(int(data.group(7) ,16))
               G_occ_response.append(int(data.group(8) ,16))
               G_occ_response.append(int(data.group(9) ,16))
               G_occ_response.append(int(data.group(10),16))
               G_occ_response.append(int(data.group(11),16))
               G_occ_response.append(int(data.group(12),16))
               G_occ_response.append(int(data.group(13),16))
               G_occ_response.append(int(data.group(14),16))
               G_occ_response.append(int(data.group(15),16))
               G_occ_response.append(int(data.group(16),16))
               G_occ_response.append(int(data.group(17),16))



def hexstr2bytearray( data_str ):
    # Convert a ascii hex string of indeterminite length to byte array
    byte_array = []
    if data_str:
        for x in xrange(0, len(data_str), 2):
            byte_array.append(int(data_str[x:x+2], 16))
    return byte_array


def binfile2bytearray( file_name ):
    # Convert a binary of indeterminite length to byte array
    data_file = open(file_name,"rb")
    byte_array = array.array('B')
    fsize = os.path.getsize(file_name)
    fsize %= 4096
    byte_array.fromfile(data_file,fsize)
    byte_list = byte_array.tolist()
    return byte_list


def occ_build_command(command, data, seq='AA'):
    global G_occ_command

    # Command is string 'XX' ascii hex representation
    # Seq     is string 'XX' ascii hex representation
    # Data    is array of data bytes

    # Empty G_occ_command buffer
    G_occ_command = []

    #Add seq num into byte 0
    G_occ_command.insert(0,int(seq,16))
    
    #Add command into byte 1
    G_occ_command.insert(1,int(command,16))

    # Add data length
    data_len_hi = (len(data)/256)
    data_len_lo = (len(data)%256)
    G_occ_command.insert(2,data_len_hi)
    G_occ_command.insert(3,data_len_lo)

    # Add data
    G_occ_command = G_occ_command + data

    # Add checksum
    cksm = occ_calc_cksm()
    G_occ_command = G_occ_command + cksm


def occ_parse_command_generic():
    print "----------------------"
    print "Sent OCC Command"
    print "----------------------"
    print "SEQUENCE: %02x" % G_occ_command[0]
    print "COMMAND : %02x" % G_occ_command[1]
    print "DATA_LEN: %02x%02x" % (int(G_occ_command[2]),int(G_occ_command[3]))
    print "DATA    :",
    data_len = int(int(G_occ_command[2])*256 + int(G_occ_command[3]))
    for byte in range((data_len -1)):
        print "%02x" % G_occ_command[byte+4],
    print ""
    print "CHECKSUM: %02x%02x"  %  (G_occ_command[ (data_len + 4) ], G_occ_command[ (data_len + 5 )  ])


def occ_parse_response_generic():
    print "----------------------"
    print "Received OCC Response"
    print "----------------------"
    print "SEQUENCE: %02x" % G_occ_response[0]
    print "COMMAND : %02x" % G_occ_response[1]
    print "RC      : %02x (%s)" % (G_occ_response[2],G_occ_resp_rc[ G_occ_response[2] ])
    print "DATA_LEN: %02x%02x" % (int(G_occ_response[3]),int(G_occ_response[4]))
    print "DATA    :",
    data_len = int(int(G_occ_response[3]) + int(G_occ_response[4]))
    for byte in range((data_len )):
        print "%02x" % G_occ_response[byte+5],
    print ""
    print "CHECKSUM: %02x%02x"  %  (G_occ_response[ (data_len + 5) ], G_occ_response[ (data_len + 6 )  ])
    print ""


def occ_trace_grabber(occid=0):
        cli.quiet_run_command("pipe \"p8Proc" + occid + ".OccComplexSlot.OccSimpleSlot.oci_space.x (p8Proc" + occid + ".OccComplexSlot.OccSimpleSlot.cpu.sym g_trac_inf) 0x6000\" \"sed 's/^p:0x........ //g' | sed 's/  ................$//g' | sed 's/ //g' | xxd -r -p> test.bin\"")
        cli.run_command("!fsp-trace -s " + os.getenv("sb") + "/../obj/trexStringFile test.bin | strings")


#----------------------------------------------------------
# Simics Helper functions
#

def occ_fsp_command(command,data="",datafile="",seq="",occid=0):
    if datafile:
        # If binary data file is passed in, append that to any other data bytes we have
        occ_build_command(command=command, data=hexstr2bytearray(data) + binfile2bytearray(datafile))
    else:
        # Do this without data bytes
        occ_build_command(command=command, data=hexstr2bytearray(data))

    # Send Command
    occ_send_command(str(int(occid or 0)))
    # Parse out the command we just sent to the screen
    occ_parse_command_generic()


def occ_fsp_response(length,occid=0):
    # Get response from SCOMs
    occ_read_command(str(int(occid or 0)))
    # Get response from SRAM -- same as above, but easier to parse
    occ_read_command_sram_direct(str(int(occid or 0)))
    # Parse out the response we just sent to the screen
    occ_parse_response_generic()


def occ_trace(occid=0):
    occ_trace_grabber(str(int(occid or 0)))




#----------------------------------------------------------
# Simics Command:  occ-fsp-cmd
#
new_command("occ-fsp-cmd", occ_fsp_command,
  args = [arg(str_t, "command"),arg(str_t, "data","?"),arg(str_t, "datafile","?"),arg(str_t, "seq","?"),arg(int_t, "occid","?")],
  alias = "",
  type = "occ-fw-module-commands",
  short = "Send FSP Command to OCC",
  doc_items = [("NOTE", "")],
  see_also = ["occ-fsp-rsp"],
  doc = """
  <b>occ-fsp-cmd</b> is used to send commands to OCC.  Checksum is automatically calculated.\n
  <i>command</i> is the hex value of the command to be sent.  i.e. "21"\n
  <i>data</i> is the hex data that should be sent.  i.e  "AA55BB66CC77"\n
  <i>datafile</i> is filename of a binary data file that should be sent.  It will be added after the data arg.  i.e. "pss.bin"\n
  <i>seq</i> is the sequence number, and it currently ignored, and defaults to AA\n
  <i>occid</i> is the OCC ID Number you want to send the command to (defaults to 0)\n
  <b>Example:</b> occ-fsp-cmd command="21" data="0B" datafile="0123.bin" \n
  will build a command like "AA2100050B0001020300E1"
""")

#----------------------------------------------------------
# Simics Command:  occ-fsp-rsp
#
new_command("occ-fsp-rsp", occ_fsp_response,
  args = [arg(int_t, "size","?"),arg(int_t, "occid","?")],
  alias = "",
  type = "occ-fw-module-commands",
  short = "Get FSP Command Response from OCC",
  doc_items = [("NOTE", "")],
  see_also = ["occ-fsp-cmd"],
  doc = """
  <b>occ-fsp-rsp</b> is used to get/parse response from OCC.
  <i>occid</i> is the OCC ID Number you want to send the command to\n
  <i>size</i>
  is the only other argument, and currently is ignored""")


#----------------------------------------------------------
# Simics Command:  occ-trace
#
new_command("occ-trace", occ_trace,
  args = [arg(int_t, "occid","?")],
  alias = "",
  type = "occ-fw-module-commands",
  short = "Get Trace from OCC",
  doc_items = [("NOTE", "")],
  see_also = ["occ-fsp-cmd"],
  doc = """
  <b>occ-trace</b> is used to get trace from OCC.
  <i>occid</i> is the OCC ID Number you want to send the command to\n""")

