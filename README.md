# protodune-adc-calib

## Introduction

This package holds the script processData for running lar on data files.
It also holds many fcl files.

### Calibration

To run protoDUNE calibration:

First event data is processed to gnerate histograms and graphs, e.g.:
```
processData calib 1194 10
```
This command uses calib.fcl to process the first 10 events for run 1194.
Results are saved in directory ./calib001194.

Then the Root scripts may be used to view these results:
```
> mkdir lyze1
> cd lyze1
> . ../../pkgs/protodune-adc-calib/setup.sh # Any location is fine
> root.exe
.
.
.
```
To draw the ROIs (pulses) for this run:
```
Root> drawRois(1194)
```
And to draw the summary histograms:
```
Root> drawSums(1194)
```

