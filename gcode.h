// gcode.h
//
// This is the primary header (.h) file for the GCode project (gcode.c)
// 
// Copyright (C) 2010, 2012 Andrew Hanes <andrewjhanes@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
// USA.
//
// See LICENSE for the full license text, or view it here:
// http://www.gnu.org/licenses/gpl-2.0.html
//
//
// (See gcode.c for project revision history)
//


#ifndef GCODE_H         // Don't define everything more than once
#define GCODE_H         //

#include <stdio.h>
#include <string.h>


// Program data constants (you may modify these)
// ----------------------------------------------------------------------------
#define OUTPUT_FILENAME "OUTPUT.txt"

// These are full fprintf() string and parameter defines. The intention is
// to use them with a "fprintf(outputFile, GCODE_HEADER);" style of line.
#define GCODE_HEADER "(Initialize)\nG20\nG90\n\n(Wire reset)\nG0 X%f U%f\nG0 Y%f V%f\n\n(Knock slew)\nG0 X%f U%f\nG0 Y%f V%f\nG0 X%f U%f\nG0 Y%f V%f\n\n(Begin airfoil upper half)\n", Y_MIN, X_MIN, Y_MIN, X_MIN, X_MAX, X_MAX, Y_MAX, Y_MAX, X_MIN, X_MIN, Y_MIN, Y_MIN
#define GCODE_FOOTER "(End airfoil lower half)\n\n(Wire reset)\n%s X%f U%f\nG0 Y%f V%f\nG0 X%f U%f\nG0 Y%f V%f\n\n(Stop)\nM30", GCODE_MOVE_COMMAND, X_MAX, X_MAX, Y_MAX, Y_MAX, X_MIN, X_MIN, Y_MIN, Y_MIN
#define GCODE_UPPERLOWER_TRANSITION "(End airfoil upper half)\n\n(Wire reset)\n%s X%f U%f\nG0 Y%f V%f\nG0 X%f U%f\nG0 Y%f V%f\n\n(Begin airfoil lower half)\n", GCODE_MOVE_COMMAND, X_MAX, X_MAX, Y_MAX, Y_MAX, X_MIN, X_MIN, Y_MIN, Y_MIN

// These are string-only defines that are used as parameters to an "%s" in the
// fprintf() call that outputs each data point line to the output file.
// They are not full fprintf() strings and parameter defines. Each data point
// in the entire airfoil will use this move command and this feedrate.
#define GCODE_MOVE_COMMAND "G1"
#define GCODE_FEEDRATE "F0.60"

// Cutter min/max dimension values (in units specified in GCode header)
// Note: These also apply to U and V axes in the output file
#define X_MIN -12.0
#define Y_MIN -12.0 
#define X_MAX 12.0
#define Y_MAX 12.0

// All data points read from the input files will be scaled by this number
#define XYUV_COORDINATE_SCALAR 5.0
// ----------------------------------------------------------------------------


// Program data constants (do not modify these)
#define TOTAL_SIDES 2               // Sides per wing (Root, Tip)
#define TOTAL_HALVES 2              // Halves per side (Upper, Lower)
#define TOTAL_DIMENSIONS_PER_HALF 2 // Dimensions per half (X, Y)

#define READONLY "r"                // File access constants
#define WRITEONLY "w"               //

// Fatal error messages
#define MESSAGE_ERROR "* Oops -- Can't "
#define MESSAGE_FILE_OPENERROR "open %s. Are all vector files present?\n"
#define MESSAGE_FILE_READERROR "read %s. The first line should be the 'Total Values'.\n"
#define MESSAGE_FILE_WRITEERROR "write to %s. Is the file in-use or the disk full?\n"
#define MESSAGE_MEMORY_ALLOCERROR "allocate memory for that many data points!\n"

// Non-fatal error messages
#define MESSAGE_WARNING "* Note: You should "
#define MESSAGE_VECTOR_CONSISTENCY "ensure all vector files list the same number of data points\n"
#define MESSAGE_VECTOR_EOF "check all vector files for the listed number of data points\n"


// Function prototypes
void OpenDataFiles();
void CloseDataFiles();
void AllocateMemory();
void FreeMemory();
void CheckVectorConsistency();
void ReadVectorData();
void OutputGCode();


// Enum values for each airfoil side
enum Side
{
    Root,
    Tip
};
char *SideToString[] = { "ROOT", "TIP" };

// Enum values for each airfoil half
enum Half
{
    Upper,
    Lower
};
char *HalfToString[] = { "UPPER", "LOWER" };

// Enum values for each airfoil axis. Note that U and V axes are
// represented here by a second Side of X and Y coordinates.
enum Dimension 
{
    X,
    Y
};
char *DimensionToString[] = { "X", "Y" };


// Each internal vector instance has these properties associated with it
typedef struct
{
    FILE *inputFile;        // File handle to the input file
    int totalValues;        // Total data point values
    float *value;           // Data point values
} Vector;


// File handle to the output file
FILE *outputFile;


#endif
// --- End of gcode.h
