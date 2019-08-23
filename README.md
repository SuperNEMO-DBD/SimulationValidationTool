# SuperNEMOSimValidation

Yorck Ramachers (Warwick), Bartosz Pyszkowski (Warwick)
Last updated August 22 2019

## Files:

- CMakeLists.txt
- SimulationTool.cxx
- SimulationTool.h

## Purpose

This tool takes two ROOT ntuple files, and generates statistics for comparisons between input and reference data files. Further tests are run on the statistics produced.
Primary use of the tests in this tool is to detect changes in Simulation Data produced by different version of the Monte-Carlo Simulation.
This will become useful in the future for tracking changes when the Simulation is updated. 

The tool is used to :
- Generate statistics from data in input and reference files
- Statistics are further used in comparison test

## Usage

After builiding the tool run the following command in the build directory to use the programme.
$ ./SimulationTool <data ROOT file> <reference ROOT file to compare to>

In order to generate comparison statistics the root input and reference files should contain branches with same names.
The output of the tool is presented in the terminal running in. The tool generates only that statics specified in the main script.
Some basic tests are present, that compare data from input and reference files.

The  statistics  generated  by  the  SimulationTool  are:  Mean,  Error  on  Mean,  Maximum  Value, Minimum  Value,  Skewness,  Standard  Deviation,  
Error  on  Standard  Deviation,  Kolmogorov-Smirnov  Test,  P-value, Chi-squared  and  number  of  degrees of freedom.

Currently,  there are 5 tests run by the SimulationTool:

1. check if input data mean lies in the range of reference data mean and one standard deviation, 
and vice versa for file order reversed;

2. check if a ratio of standard deviation errors is between 0.99 and 1.01 for both files;

3. check if Mean values of input file are within range of reference mean values and their errors, 
and vice versa for file order reversed;

4. check if Maximum of input file is larger than Minimum of reference file, and vice versa for file order reversed;

5. check if P-value is larger than 0.99.

Note: To use this tool the branches have to be saved in a Tree titled "SimValidation".
