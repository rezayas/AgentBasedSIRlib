#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import SIRsimPyModule

if (len(sys.argv) < 13 or len(sys.argv) > 13):
    print """\
This script will run the SIRSim

You need to provide 12 arguments

Usage: RunSIRSim runType fileName nTrajectores λ Ɣ nPeople ageMin ageMax ageBreak tMax Δt pLength
"""
    sys.exit(0)

configureArgs = sys.argv[1:13];
runArgs = sys.argv[0:0];

runner = SIRsimPyModule.configure(configureArgs)

run = SIRsimPyModule.Run(runArgs)

print(sys.argv)