DIY Airfoils you say? Put numbers in, get G-code out!
=================


[![C](https://img.shields.io/badge/language-c-blue.svg)](https://github.com/gregkrsak/gcode/blob/master/gcode.c)


This project outputs G-code that is used to control a 4-axis hot wire cutter, employed to cut a wing / airfoil from a physical block of foam. Effectively, it is a slicer for a particular arrangement of raw-coordinate data files.

* [What is G-code?]

I wrote this for a family member who was, at the time, working in airfoil design. The code is designed to be cross-platform (Windows, OS X, and Linux).

Quick Rundown
-------------

Eight input files are provided to the program; each input file is formatted in the following manner:

```
   <integer:total_values_to_follow>
   <float:value1>
   <float:value2>
   <...>
   <float:valueN>
```

So that an input file may look like:

```
   3
   -1.000000
   0.989000
   1.200000
```

Where "3" is the number of values to follow, with the three following values on separate lines.

Each input file is named to correspond with a technical section of the wing / airfoil (Root & Tip; Upper & Lower halves; X & Y coordinates). The input file names are, at this time, constant and should not be changed. In other words, all of the input files must be present and reasonably formatted for the program to function.

Input files:

   "ROOTUPPERX", "ROOTUPPERY", "ROOTLOWERX", "ROOTLOWERY",
   "TIPUPPERX", "TIPUPPERY", "TIPLOWERX", "TIPLOWERY"

Output file:

   "OUTPUT.txt"

  [What is G-code?]: http://en.wikipedia.org/wiki/G-code
