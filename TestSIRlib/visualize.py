#!/usr/bin/env python
# -*- coding: utf-8 -*-
import agent_based

# Visualize
agent_based.read("output/Age-distribution of cases - data.csv", "output/Age-distribution of cases - model.csv", "output/Weekly cases - data.csv", "output/Weekly cases - model.csv", "output/test-susceptible.csv", "output/test-infected.csv", "output/test-recovered.csv")

agent_based.plot("output/graphs.pdf")
