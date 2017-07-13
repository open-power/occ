#!/usr/bin/env python3

# (C) Copyright IBM Corporation 2017
# Wael El-Essawy
# welessa@us.ibm.com

from __future__ import print_function

import pandas as pd
import numpy as np

import re

import argparse
import requests

import struct
import ctypes

import codecs

import sys
import os.path

from datetime import datetime
import time

import requests
import requests
from requests.packages.urllib3.exceptions import InsecureRequestWarning

requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

# Global variables
global debug_mode    # display debug information in the script output. Enabled by --debug, -d switch
global csv_format    # display output in csv format. Enabled by --csv, -c switch

debug_mode = False
csv_format = False

# Maximum number of sensors to be fetched from the OCC in one call
MAX_SENSORS = 10

# BMC class: manages logging in and out
class BMC:
    def __init__(self, server):
        self.url = "https://{0}/".format(server)
        self.session = requests.Session()
        self.login()

    def login(self):
        r = self.session.post(self.url + 'login',
                              json={'data': ['root', '0penBmc']},
                              verify=False)
        j = r.json()
        if j['status'] != 'ok':
            raise Exception("Failed to login: \n" + r.text)


    def logout(self):
        r = self.session.post(self.url + 'logout',
                              json={'data': []},
                              verify=False)
        j = r.json()
        if j['status'] != 'ok':
            raise Exception("Failed to logout: \n" + r.text)


# Amester Class
class AMESTER:

    def __init__(self, bmc):
        # Get session and url from the BMC object
        self.session = bmc.session
        self.url = bmc.url

        # Ideltify the OCCs list, and exit if there are no Active OCCs in the system
        self.identify_occs()

        # Probe AME for constants: Determine the number of OCC sensors, AME version, etc
        self.get_ame_constants()

        # Probe AME for number of OCC Parameters
        self.get_num_parameters()

    def identify_occs(self):
        r = self.session.get(self.url + '/org/open_power/control/enumerate',
                             verify=False)
        j = r.json()
        if j['status'] != 'ok':
            raise Exception("Failed to identify OCCs \n" + r.text)
        self.occ_list = list(j['data'].keys())
        self.occ_list.sort()
        if not self.occ_list:  # No OCC Objects identified on the DBUS, OCCs may not be active
            raise Exception("No Active OCCs found! \n")

    # Get number of OCC Sensors
    def get_ame_constants(self, occ=0):
        if occ > len(self.occ_list):
            raise Exception("Invalid occ ID")

        r = self.session.post(self.url + self.occ_list[occ] + '/action/Send',
                         json={'data': [[0x41, 0, 3, 0x3C, 0, 0x1C]]},  # 0x1c:
                         verify=False)
        [AME_API_MAJ, AME_API_MIN, AME_VERSION_MAJ, AME_VERSION_MIN] = r.json()['data'][5:9]

        if(debug_mode):
            print("AME API",     str(AME_API_MAJ)     + '.' + str(AME_API_MIN),
                  "AME Version", str(AME_VERSION_MAJ) + '.' + str(AME_VERSION_MIN))

        ame_year = r.json()['data'][9] * 256 + r.json()['data'][10]
        [ame_month, ame_day] = r.json()['data'][11:13]
        if(debug_mode):
            print("AME Date:", str(ame_month) + '/' + str(ame_day) + '/' + str(ame_year))

        self.number_of_sensors = r.json()['data'][13] * 256 + r.json()['data'][14]
        if(debug_mode):
            print("# Sensors:", self.number_of_sensors)

    # Get number of OCC Parameters
    def get_num_parameters(self, occ=0):
        if occ > len(self.occ_list):
            raise Exception("Invalid occ ID")

        r = self.session.post(self.url + self.occ_list[occ] +'/action/Send',
                                           json={'data': [[0x41, 0, 3, 0x3C, 0, 0x40]]},  # 0x40:
                                           verify=False)

        self.number_of_parameters = r.json()['data'][5] * 256 + r.json()['data'][6]
        if(debug_mode):
            print("# Parameters:", self.number_of_parameters)



class Sensor:
    def __init__(self, amester, occ=0, fname="sensors_info.csv"):
        if occ > len(amester.occ_list):
            raise Exception("Invalid occ ID")
        self.session = amester.session
        self.url = amester.url
        self.number_of_sensors = amester.number_of_sensors
        self.occ = occ
        self.occ_list = amester.occ_list
        self.occ_path = self.occ_list[occ]

        # populate the Sensor Info DataFrame
        if os.path.isfile(fname):
            self.Info = pd.read_csv(fname)
            if (debug_mode):
                print("Loading OCC Sensors from", fname)
        else:
            self.get_info()
            self.Info.to_csv(fname, index_label='GSID')
            if (debug_mode):
                print("Saving OCC Sensors Info to", fname)

    def is_valid(self, sensor):
        return (sensor in self.Info.values[:, 0])


    # reads a list of sensors from an OCC
    def get_sensor_list(self, sensor_ids, occ):
        # confirm that all sensor IDs are valid
        # and convert IDs to bytes
        sensor_ids_bytes = []
        for sensor_id in sensor_ids:
            if not self.is_valid_id(sensor_id):
                raise Exception("Invalid Sensor ID!", sensor_id)
            sensor_ids_bytes = sensor_ids_bytes + [sensor_id>>8, sensor_id%256]


        r = self.session.post(self.url + self.occ_list[occ] + '/action/Send',
                              json={'data': [
                                  [0x41, (3 + len(sensor_ids_bytes))>>8, 3 + len(sensor_ids_bytes), 0x3C, 0, 0x7]
                                   + sensor_ids_bytes]},
                              verify=False)
        if r.json()['status'] != 'ok':
            raise Exception("Failed to get OCC Sensors" ,
                            ','.join(str(e) for e in sensor_ids) + ": \n" + r.text)
        return r




    # read a list of sensors from the OCC.
    def get(self, sensor_ids, occ):
        # number of sensors requested in the sensor_ids list
        num_sensors = len(sensor_ids)

        # list must be less than or equal to the MAX_SENSORS constant
        assert num_sensors <= MAX_SENSORS

        # get sensor list from the OCC
        r = self.get_sensor_list(sensor_ids, occ)

        packet_length = int.from_bytes(r.json()['data'][3:5], byteorder='big')
        sensors_array = bytearray(r.json()['data'][5:packet_length + 5])
        sensor_tuples = struct.unpack('>' + 'IIQHHHH' * num_sensors, bytes(sensors_array))

        field_names = ["timestamp", "updates", "accumulator", "value", "min", "max", "status"]
        sensor_list = [sensor_tuples[i: i+len(field_names)]
                       for i in range(0, len(sensor_tuples), len(field_names))]
        Sensors = pd.DataFrame(sensor_list, columns=field_names, index = sensor_ids)

        return Sensors


    def get_id(self, snsor):
        if self.is_valid(snsor):
            sensor_id = self.Info.loc[self.Info.name == snsor].index[0]
            return(sensor_id)

    # Is "sensor" a valid sensor name?
    def is_valid(self, snsor):
        return (snsor in self.Info.values[:, 1])

    def is_valid_id(self, sensor_id):
        return(sensor_id in self.Info.index)


    # OCC Pass through command to get sensors info starting at parameter_id
    def get_sensor_info(self, sensor_id):
            r = self.session.post(self.url + self.occ_path + '/action/Send',
                              json={'data': [
                                  [0x41, 0, 6, 0x3C, 0, 0x25,
                                   sensor_id / 256, sensor_id % 256, 5]]},
                              verify=False)
            if debug_mode:
                print("data: [0x41, 0, 6, 0x3C, 0, 0x25,", int(sensor_id / 256), ",", int(sensor_id) % 256, "5]",
                      self.url + self.occ_path + '/action/Send')

            if r.json()['status'] != 'ok':
                raise Exception("Failed to get OCC sensor " + str(sensor_id) + ": \n" + r.text)

            return(r)


    # Get Sensors Inf List
    def get_info(self):
        sensors_array = bytearray()
        start_sensor = 0
        start_time = datetime.now()

        if(debug_mode):
            print("Collecting Sensors Info from OCC:\n", start_sensor)

        while start_sensor < self.number_of_sensors:
            r = self.get_sensor_info(start_sensor)

            # print(r.text)
            packet_length = r.json()['data'][3] * 256 + r.json()['data'][4]
#            names = str(bytearray(r.json()['data'][5:packet_length + 5]),'utf8', errors='ignore')
            names = bytearray(r.json()['data'][5:packet_length + 5])
            sensors_array.extend(names)
            start_sensor = start_sensor + \
                           len(re.findall(r'\w+\0[\\ \/ \% \# A-z]+\0',
                                          str(names,'utf8', errors='ignore')))
            if (debug_mode):
                print(start_sensor)
            # workaround for the OCC pass through commands timeout issue
            # https://github.com/openbmc/openbmc/issues/1700
            time.sleep(1)

        if(debug_mode):
            print(start_sensor)

#        sensors_array.extend(bytearray(b'\x00'))

        delta = datetime.now() - start_time
#        print("elapsed time in microseconds", delta.microseconds)
#        sensors_array = sensors_array[:-2]
        sensor_tuples = zunpack('>' + 'zzII' * (self.number_of_sensors),
                                str(sensors_array,'windows-1252'))

        fields_names = ["name", "unit", "frequency", "scale"]
        sensor_list = [sensor_tuples[i:i + len(fields_names)]
                       for i in range(0, len(sensor_tuples), len(fields_names))]
        self.Info = pd.DataFrame(sensor_list, columns=fields_names)

        # convert Amester's NeM format into numerical samples/sec rate
        self.Info.frequency = self.Info.frequency.map(
            lambda x: ((x >> 8) * (10.0 ** ctypes.c_int8(x & 0xff).value)))
        self.Info.scale = self.Info.scale.map(
            lambda x: ((x >> 8) * (10.0 ** ctypes.c_int8(x & 0xff).value)))

        # Cleanup Sensor Info Strings
        self.Info['name'] = self.Info['name'].str.decode("utf-8")
        self.Info['unit'] = self.Info['unit'].str.decode("utf-8")



class Parameter:
    def __init__(self, amester, occ=0, fname="parameters_info.csv"):
        if occ > len(amester.occ_list):
            raise Exception("Invalid occ ID")
        self.session = amester.session
        self.url = amester.url
        self.number_of_parameters = amester.number_of_parameters
        self.occ = occ
        self.occ_list = amester.occ_list
        self.occ_path = self.occ_list[occ]

        self.type_length = np.array([1, 2, 4, 8, 1, 2, 4, 8, 1, 1])
        self.special_param_lengths = dict()

        if os.path.isfile(fname):
            self.Info = pd.read_csv(fname)
            if(debug_mode):
                print("Loading OCC parameters from", fname)
        else:
            self.get_info(amester.occ_list[self.occ])
            self.Info.to_csv(fname, index_label='PARM ID')
            if (debug_mode):
                print("Saving OCC Parametets Info to", fname)


    # read parameter_id
    def get(self, param_id, occ):
        r = self.session.post(self.url + self.occ_list[occ] + '/action/Send',
                              json={'data': [
                                  [0x41, 0, 9, 0x3C, 0, 0x42, 0, 0, 0, 0, param_id / 256, param_id % 256]]},
                              verify=False)

        if r.json()['status'] != 'ok':
            raise Exception("Failed to get OCC parameter " + str(param_id) + ": \n" + r.text)

        if not self.is_valid_id(param_id):
            raise Exception("Invalid Parameter ID!")

        packet_length = r.json()['data'][3] * 256 + r.json()['data'][4]
        param_name = self.Info['name'][param_id]
        param_length = self.Info['vlength'][param_id]
        param_type = self.Info['type'][param_id]

        if packet_length != (param_length * self.type_length[param_type]):
            print(r.json())
            print("Invalid Packet Parameter Size: parameter:", param_name,
                  "parameter size", param_length, "packet size:", packet_length)
            raise Exception("Parameter Info and actual parameter packet Mismatch. \nHint: detele parameters_info.csv")

        # Fix Packet Length Bug
        # https://github.com/openbmc/openbmc/issues/1698
        r.json()['data'] = r.json()['data'][:5 + packet_length]

        value_str = ""

        # parameter values position identifiers (based on parameter type)
        for i in range(0, param_length):
            offset = i * self.type_length[param_type]
            start = 5 + offset
            end = 5 + offset + self.type_length[param_type]

            if self.Info['type'][param_id] <4:      # Unsigned Value
                value = int.from_bytes(r.json()['data'][start:end],
                                             byteorder='big', signed=False)
            elif self.Info['type'][param_id] < 8:                                   # Signed Value
                value = int.from_bytes(r.json()['data'][start:end],
                                       byteorder='big', signed=True)
            elif self.Info['type'][param_id] == 8:  # String
                value = str(r.json()['data'][start:end], "utf8")
                print(param_name + ":", string_value, end='')
            else:  # Raw
                value = int.from_bytes(r.json()['data'][start:end],
                                             byteorder='big', signed=False)
            value_str = value_str + str(value) + '|'

        return value_str[:-1]


    def get_id(self, param):
        if self.is_valid(param):
            param_id = self.Info.loc[self.Info.name == param].index[0]
            return(param_id)

    # Is "param" a valid parameter name string?
    def is_valid(self, param):
        return (param in self.Info.values[:, 1])

    # Is "param_id" a valid parameter ID number?
    def is_valid_id(self, param_id):
        return(param_id in self.Info.index)

    # OCC Pass through command to get parameters info starting at parameter_id
    def get_parameter_info(self, parameter_id):
            r = self.session.post(self.url + self.occ_path + '/action/Send',
                              json={'data': [
                                  [0x41, 0, 5, 0x3C, 0, 0x41,
                                   parameter_id / 256,
                                   parameter_id % 256]]},
                              verify=False)
            if debug_mode:
                print("data: [0x41, 0, 5, 0x3C, 0, 65,", int(parameter_id / 256), ",", int(parameter_id) % 256, "]",
                      self.url + self.occ_path + '/action/Send')

            if r.json()['status'] != 'ok':
                raise Exception("Failed to get OCC parameter " + str(parameter_id) + ": \n" + r.text)

            return(r)

    # Get Parameters Inf List
    def get_info(self, occ_path):
        parameters_array = bytearray()

        start_parameter = 0

        if(debug_mode):
            print("Collecting Parameters Info from OCC:\n", start_parameter)
        while start_parameter < self.number_of_parameters:
            r = self.get_parameter_info(start_parameter)

            packet_length = r.json()['data'][3] * 256 + r.json()['data'][4]
            names = bytearray(r.json()['data'][5:packet_length + 5])

            match = re.search(r'(\w+)\x00\x09', str(names, "utf8"))           # Raw Sensor
            if match:
                length_pos = match.end() + 5
                (length,) = struct.unpack(">I", names[length_pos:length_pos + 4])
                self.special_param_lengths[match.group(0)[:-2]] = length
                names = names[:match.end()] + names[match.end()+4:]

            parameters_array.extend(names)

#            print (names, parameters_array)
            start_parameter = start_parameter + len(re.findall(r'\w+', str(names,'utf8', errors='ignore')))

            if(debug_mode):
                print (start_parameter)
            # Delay inserted to overcome occ DBUS controller bug
            # https://github.com/openbmc/openbmc/issues/1700
            time.sleep(1)

        if(debug_mode):
            print (start_parameter)
#        delta = datetime.now() - start
#        print ("elapsed time in microseconds", delta.microseconds)

        parameter_tuples = zunpack('>'+'zBBI'*(self.number_of_parameters),
                                   str(parameters_array,'windows-1252'))
        parameter_fields = ["name", "type", "Mode", "vlength"]
        parameter_list = [parameter_tuples[i:i + len(parameter_fields)]
                          for i in range(0, len(parameter_tuples), len(parameter_fields))]
        self.Info = pd.DataFrame(parameter_list, columns=parameter_fields)

        # Cleanup Parameter Info Strings
        self.Info['name'] = self.Info['name'].str.decode("utf-8")
        self.Info['length'] = ""
        for special_param in self.special_param_lengths.keys():
            param_id = self.Info.loc[self.Info.name == special_param].index[0]
            self.Info.loc[param_id,'length'] = self.special_param_lengths[special_param]
#            self.Info['length'][param_id] = self.special_param_lengths[special_param]


def zunpack (format, buffer) :
    while True :
        pos = format.find ('z')
        if pos < 0:
            break
        start = struct.calcsize(format[:pos])
        str_len = buffer[start:].find('\0')
        if str_len < 0 :
            print (pos, start, str_len)
            str_len = str_len + 16
        format = '%s%dsx%s' % (format[:pos], str_len, format[pos+1:])
#    print (pos, format, codecs.encode(bytes(buffer), 'hex'), codecs.encode(bytes(buffer), "utf8"))
    if (debug_mode):
        print("parameters hex string length:", len(buffer),
              "parameters format length:", len(format))
    return struct.unpack (bytes(format, "windows-1252"), bytes(buffer, "windows-1252"))



# Get Parameters, associated with p command
def get_parameter(args):
    field_names = ["occ", "parameter", "parameter_id", "value"]
    parameter_measurement = pd.DataFrame(columns=field_names)
    compact_parameter_measurement = dict()

    # if no parameters list provided, get all parameters
    if not args.param:
        args.param = parameter.Info['name']

    parameters_chunk = [args]

    for parameter_item in args.param:
        # Paramter ID requested
        if parameter_item.isdigit():
            parameter_id = int(parameter_item)

            # if Parameter ID is not a valid one print an error and return
            if not parameter.is_valid_id(parameter_id):
                print("Invalid OCC Parameter ID")
                return

            parameter_name = parameter.Info['name'][parameter_id]

        # Parameter name requested, is it a valid parameter name?
        elif parameter.is_valid(parameter_item):
            parameter_id = np.asscalar(parameter.get_id(parameter_item))
            parameter_name = parameter_item

        # An invalid parameter string. print error and return
        else:
            print("Invalid OCC Parameter String")
            return

        # get parameter measurement from the specified OCC
        value = parameter.get(parameter_id, args.occ)

        # Check if the parameter contains an address,
        # and if so store it in hexadecimal format
        address = re.search(r'addr', parameter_name, re.IGNORECASE)
        if address:
            value = hex(int(value))

        # csv format requested, store parameters data
        # (both in conventional table and compact formats)
        if csv_format:
            parameter_measurement = parameter_measurement.append(
                pd.Series([args.occ, parameter_name, parameter_id, value],
                          index=field_names), ignore_index=True)

            compact_name = 'OCC' + str(args.occ) + '|' + \
                           parameter_name + '|' + str(parameter_id)
            compact_parameter_measurement[compact_name] = str(value)

        # csv format is not requested, print parameters one by one
        else:
            print(parameter_name + "(OCC" + str(args.occ) + ":",
                  parameter_id, ")" + ": [", value, "]")


        # workaround for the OCC pass through commands timeout issue
        # https://github.com/openbmc/openbmc/issues/1700
        time.sleep(1)

    # CSV format requested, print stored parameters measurement and data
    if csv_format:
        # uncomment next LOC if you prefer printing conventional (non compact) table
        #sensor_measurement.to_csv("/dev/stdout")
        keys_str = ','.join(compact_parameter_measurement.keys())
        values_str = ','.join(compact_parameter_measurement.values())
        print(keys_str)
        print(values_str)


# Get Sensors, associated with s command
def get_sensor(args):
    field_names = ["occ", "sensor", "sensor_id", "value", "unit"]
    sensor_measurement = pd.DataFrame(columns=field_names)
    compact_sensor_measurement = dict()

    # if no sensor list provided, get all sensors
    if not args.sensor:
        args.sensor = sensor.Info['name']

    for sensor_item in args.sensor:
        # Sensor ID requested
        if sensor_item.isdigit():
            sensor_id = int(sensor_item)

            # if ID is not a valid GSID, print an error and return
            if not sensor.is_valid_id(sensor_id):
                print("Invalid OCC Sensor ID")
                return

            sensor_name = sensor.Info['name'][sensor_id]

        # Sensor name requested, is it a valid sensor name?
        elif sensor.is_valid(sensor_item):
            sensor_id = np.asscalar(sensor.get_id(sensor_item))
            sensor_name = sensor_item

        # sensor string specified in command line is invalid.
        else:
            print("Invalid OCC Sensor name string")
            return

        # get sensor measurement from the specified OCC
        value = sensor.get(sensor_id, args.occ)

        # csv format requested, store sensors data
        # (both in conventional table and compact formats)
        if csv_format:
            sensor_measurement = sensor_measurement.append(
                pd.Series([args.occ, sensor_name,
                           sensor_id, value, sensor.Info['unit'][sensor_id]],
                          index=field_names), ignore_index=True)

            compact_name = 'OCC' + str(args.occ) + '|' + \
                           sensor.Info['name'][sensor_id] + '|' + \
                           str(sensor_id) + '|' + sensor.Info['unit'][sensor_id]
            compact_sensor_measurement[compact_name] = str(value)

        # csv format is not requested, print sensors one by one
        else:
            print(sensor_name + "(OCC" + str(args.occ) + ":", sensor_id, ")" \
                  + ": [ ", value, sensor.Info['unit'][sensor_id], "]")

        # workaround for the OCC pass through commands timeout issue
        # https://github.com/openbmc/openbmc/issues/1700
        time.sleep(1)

    if csv_format:
        # uncomment next LOC if you prefer printing conventional (non compact) table
#       sensor_measurement.to_csv("/dev/stdout")  # Print conventional table
        keys_str = ','.join(compact_sensor_measurement.keys())
        values_str = ','.join(compact_sensor_measurement.values())
        print(keys_str)
        print(values_str)

def compact_names(x):
    return 'OCC' + str(x['occ']) + '|' + x['name'] + '|' + str(x['id']) + '|' + x['unit']


# Get Sensors, associated with s command
def get_sensor_list(args):

    # if no sensor list provided, get all sensors
    if not args.sensor:
        args.sensor = sensor.Info['name']

    measurements = pd.DataFrame()
    all_sensor_ids = []

    # split the requested sensor list into
    # chunks of at most MAX_SENSORS sensors
    for i in range(0, len(args.sensor), MAX_SENSORS):
        chunk = args.sensor[i: i + MAX_SENSORS]

        # verify sensor names/IDs, and arrange produce a tuple of names and IDs
        sensor_tuples = get_ids(sensor, chunk)
        sensor_ids   = list( x[0] for x in sensor_tuples)
        all_sensor_ids = all_sensor_ids, sensor_ids
        sensor_names = list( x[1] for x in sensor_tuples)
        sensor_units = pd.Series(sensor.Info.loc[sensor_ids,'unit'], index = sensor_ids)

        # Get the sensor list from the specified OCC
        Sensors = sensor.get(sensor_ids, args.occ)

        # Add Sensor meta data from Sensors Info dataframe. and
        Sensors['name'] = pd.Series(data=sensor_names, index = sensor_ids)
        Sensors['occ']  = pd.Series(int(args.occ), index = sensor_ids)
        Sensors['id']   = pd.Series(data=sensor_ids, index = sensor_ids)
        Sensors['unit'] = sensor_units

        # append read sensor chunk to measurements
        if measurements.empty:
            measurements = Sensors
        else:
            measurements = pd.concat([measurements, Sensors])

        time.sleep(1)

    # csv format requested, store sensors data in compact formats
    if csv_format:
        measurements['compact'] = measurements.apply(lambda x: compact_names(x), axis=1)

    # csv format is not requested, print sensors one by one
    else:
        measurements.to_csv('/dev/stdout')



    if csv_format:
        # uncomment next LOC if you prefer printing conventional (non compact) table
        #       Sensors.to_csv("/dev/stdout")     # Print conventional table format, with much more data
        keys_str = ','.join(measurements['compact'].astype(str))
        values_str = ','.join(measurements['value'].astype(str))
        print(keys_str)
        print(values_str)



# takes a mixed list of sensors/parameters IDs or names
# returns an ordered list of tuples of sensors/parameters IDs and names
# First tuple element is the sensor/parameter ID, second element is sensor/parameter name
def get_ids (obj_instance, element_list):
    # initialize list of tuples
    result = []
    for element in element_list:
        # Sensor ID requested
        if element.isdigit():
            id = int(element)

            # if ID is not a valid GSID/Parameter ID, print an error and return
            if not obj_instance.is_valid_id(id):
                raise Exception("Invalid OCC", obj_instance.__name__, "ID: ", element)

            # Sensor or Parameter name
            name = obj_instance.Info['name'][id]

        # Sensor name requested, is it a valid sensor name?
        elif obj_instance.is_valid(element):
            id = np.asscalar(obj_instance.get_id(element))
            name = element

        # sensor/parameter string specified in command line is invalid.
        else:
            raise Exception("Invalid OCC", obj_instance.__name__, "name string", element)

        # append the ID and name to the results list of tuples
        result = result + [(id, name)]
    return result



# q to quit interactive mode
def quit_script(args):
    quit()


# Interactive command processor
def interactive():
    while True:
        command = input('AMESTER$: ')
        try:
            argstr = parser.parse_args(command.split())
        except SystemExit:
            # trap argparse error message
            print('error')
            break
        if 'func' in argstr:
            argstr.func(argstr)
        else:
            print ('done')
            break


# Command Line Parser Setup
# Note: the help/description fields offer a
#       brief description for each parameter
parser = argparse.ArgumentParser(description='Amester Script utilizing OpenBMC OCC Pass Thru')
parser.add_argument('-H', '--host_ip', help='hostname or IP of BMC, default is w56', type=str, default='w56',
                    required=False) #optional parameter

parser.add_argument( '-i', '--interactive', action="store_true",
                     help='Run Amester Script in interactive mode')

parser.add_argument( '-d', '--debug', action="store_true",
                     help='print debugging/trace information')

parser.add_argument( '-c', '--csv', action="store_true",
                     help='display sensor/parameter readings in compact csv format')


subparsers = parser.add_subparsers()


parm = subparsers.add_parser(
    'p', help='get parameter list from OCC')
parm.add_argument('param', nargs="*", help='The parameter IDs/Names to view: 0 args to get all parameters')
parm.set_defaults(func=get_parameter)

snsr = subparsers.add_parser(
    's', help='get sensor list from OCC')
snsr.add_argument('sensor', nargs="*", help='The Sensor ID/Name to view: 0 args to get all sensors')
snsr.set_defaults(func=get_sensor_list)

quit_cmd = subparsers.add_parser( 'q', help='quit')
quit_cmd.set_defaults(func=quit_script)


for occ_parser in [parm, snsr]:
    occ_parser.add_argument('-o', '--occ', help='target OCC, default is 0 (OCC0)', type=int, default='0',
                        required=False)     # optional OCC target id for sensors and parameters commands

args = parser.parse_args()


if(args.debug):
    debug_mode = True

if(args.csv):
    csv_format = True

# Initialze system Objects
System = BMC(server=args.host_ip)

Amester = AMESTER(System)

parameter = Parameter(Amester)
sensor = Sensor(Amester)

# Excute commands passed through the command line
if 'func' in args:
    args.func(args)
else:
    parser.print_help()


# run script in interactive mode
if(args.interactive):
    interactive()
