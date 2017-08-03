#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import SIRsimPyModule
import agent_based


if (len(sys.argv) < 13 or len(sys.argv) > 13):
    print """\
This script will run the SIRSim

You need to provide 12 arguments

Usage: RunSIRSim runType fileName nTrajectores λ Ɣ nPeople ageMin ageMax ageBreak tMax Δt pLength
"""
    sys.exit(0)

runner = SIRsimPyModule.configure(sys.argv[2],int(sys.argv[3]),float(sys.argv[4]),float(sys.argv[5]),long(sys.argv[6]),int(sys.argv[7]),int(sys.argv[8]),int(sys.argv[9]),int(sys.argv[10]),int(sys.argv[11]),int(sys.argv[12]))

SIRsimPyModule.run(int(sys.argv[1]))
SIRsimPyModule.write()

# Visualize
agent_based.read("output/Age-distribution of cases - data.csv", "output/Age-distribution of cases - model.csv", "output/Weekly cases - data.csv", "output/Weekly cases - model.csv", sys.argv[2] + "-susceptible.csv", sys.argv[2] + "-infected.csv", sys.argv[2] + "-recovered.csv")

agent_based.plot("output/graphs.pdf")
