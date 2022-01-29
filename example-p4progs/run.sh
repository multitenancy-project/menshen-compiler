#!/bin/bash

# generate configurations of stateful memory, dummy p4 program
$P4C_FPGA --outputfile stateconf.txt --conffile allocate.txt --statefulconf 1
# sys program
$P4C_FPGA ./sys.p4 --vid 15 --outputfile confsys.txt --conffile allocate.txt --onlysys 1
# user program
$P4C_FPGA ./calc.p4 --vid 1 --outputfile conf1.txt --conffile allocate.txt

