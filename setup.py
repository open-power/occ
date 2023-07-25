# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: setup.py $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2020,2023
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
import os.path
import os
from setuptools.command.install import install
"""
 Setuptools is an open source package.
 Documentation on setuptools can be found on the web.

 OCC Usages:

 Command to run from OCC repo:
   python3 setup.py bdist_wheel

   dist/OCC-0.1-py3-none-any.whl will be produced.

   If desired, export PELTOOL_VERSION can be defined
   to customize the version before running setup.py
     export PELTOOL_VERSION=6.7.1006

 To install the OCC wheel:
   pip3 install --user OCC-0.1.py3-non-any.whl

 If necessary, setup the environment before starting:
   scl enable rh-python36 "bash"

 At this time the content of this setup.py consists
 ONLY of packaging the occStringFile since the location
 of the binary resides ONLY in the OCC repository.

 The source PEL parsers are properly packaged in the
 Hostboot source PEL parsing whl.

"""
from setuptools import setup


# Handy debug environment tips
# HERE = os.path.abspath(os.path.dirname(__file__))
# custom_data_files is a list of tuples

custom_data_files = [ ( 'occ_data', ['obj/occStringFile']) ]

def check_environment_files():
    """
    Check the environment for the needed files

    OCC setup.py is invoked in two contexts:
    1 - op-build, where the occStringFile exists, post build
    2 - OpenBMC, where the occStringFile does NOT exist
        OpenBMC clones a clean OCC repo (source only)

    setup.py will fail if data_files do not exist,
    so if we encounter a missing file, clear the
    expectation.
    """

    # Validate that the custom_data_files really exist
    for i in custom_data_files:
        for x in i[1]:
            if not os.path.isfile(x):
                custom_data_files.clear()
                return

class CustomOCCCommand(install):
    """
    Subclass the install command

    This allows the capability to add custom build
    steps.
    """
    def run(self):
        # First run the custom environment setup needed.
        # Then call the standard install once the files
        # have been properly copied to their packaging names.
        check_environment_files()
        install.run(self)

setup(
    name            = "OCC",
    cmdclass        = {'install': CustomOCCCommand},
    version         = os.getenv('PELTOOL_VERSION', '0.1'),
    data_files      = custom_data_files,
)
