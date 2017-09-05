// gcode.c
//
// Takes separate airfoil coordinate files and outputs a single GCode file 
// for CAM processing
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
// Revision History:
//
// v0.9.4, 9/4/2017
//     - [3ebee92] Fixed GitHub issue #2 (Valgrind Errors) related to
//       'filename' scratch variable.
// v0.9.3, 2/1/2012
//     - Moved project to GitHub / git
//     - Updated copyright and copyright holder's contact information
//     - Renamed "readme.txt" to "README" and added additional content
//     - Added "LICENSE" file, containing GPLv2 license
// v0.9.2, 12/7/2010
//     - Created Google Code / Mercurial project for version control at
//       http://gcode.googlecode.com
//     - Project is now covered under the GNU General Public License v2
//       (full version included as #define LICENSE="..." in gcode.h)
//     - Renamed source file "main.c" to "gcode.c"
// v0.9.1, 12/6/2010
//     - Modified the final Wire Reset commands to prevent the wire from
//       cutting the airfoil in half
//     - Successful CAM simulation of separate Upper and Lower airfoil halves
//     - Added coordinate scalar to X, Y, U and V axes. The program will scale
//       all data points by this number as they are read from the input files
// v0.9.0, 12/3/2010
//     - Refactored all previous versions to Minor Version x.8.x (from x.9)
//     - Added concept of "Half" and associated enum to represent Upper and
//       Lower airfoil halves for each Side
//     - Modified input file processing to accept eight files (vs. six).
//       The filenames are: ROOTUPPERX, ROOTUPPERY, ROOTLOWERX, ROOTLOWERY,
//       TIPUPPERX, TIPUPPERY, TIPLOWERX, TIPLOWERY
//     - Feedrate is now fixed to a constant value
//     - Renamed axis[][][] to thisVector[][][]
// v0.8.2, 12/1/2010
//     - Added Revision History
//     - Adjusted source code tabs and line breaks to improve readability,
//       and revised comments
//     - Refactored the concept of "Sides" to replace Upper and Lower
//       with Root and Tip
//     - Increased F dimension decimal precision from n.n to n.nn
// v0.8.1, 11/30/2010
//     - Added source code comments
//     - Adjusted output header and footer string processing to be somewhat
//       more flexible
//     - Verified to compile and run with GCC on Windows, Linux, and Mac OS X
//     - First successful CAM simulation of GCode output
// v0.8.0 (source unversioned)
//     - Initial Proof-of-concept
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcode.h"


// Declare the variable that holds the vector data
Vector thisVector[TOTAL_SIDES][TOTAL_HALVES][TOTAL_DIMENSIONS_PER_HALF];


////////// MAIN PROGRAM BLOCK //////////
int main (int argc, const char *argv[])
{
    OpenDataFiles();
    atexit(CloseDataFiles);
        
    CheckVectorConsistency();
        
    AllocateMemory();
    atexit(FreeMemory);
        
    ReadVectorData();
    OutputGCode();
                
    // Normal program exit
    return(EXIT_SUCCESS);
}
////////////////////////////////////////


// Function name: OpenDataFiles()
// Purpose: Opens all input and output files requred by the program. This
//          function also reads the first numerical value from the file, which
//          should be the total number of values to follow.
//
void OpenDataFiles()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;

    char *filename; // This local variable is a scratchpad for constructing
                    // a dynamic filename
        
        
    // Open the output file
    outputFile = fopen(OUTPUT_FILENAME, WRITEONLY);
                
    // Open all vector input files
    for (thisSide = Root; thisSide <= Tip; thisSide++)
    {
        for (thisHalf = Upper; thisHalf <= Lower; thisHalf++)
        {
            for (thisDimension = X; thisDimension <= Y; thisDimension++)
            {
                // Allocate memory for the local variable which will hold
                // the input filename
                filename = (char *)malloc(strlen(SideToString[thisSide]) +
                           strlen(HalfToString[thisHalf]) +
                           strlen(DimensionToString[thisDimension]) + 1);
                // Populate that variable with a dynamically-generated
                // input filename
                strcpy(filename, SideToString[thisSide]);
                strcat(filename, HalfToString[thisHalf]);
                strcat(filename, DimensionToString[thisDimension]);
                // Open the file
                thisVector[thisSide][thisHalf][thisDimension].inputFile =
                  fopen(filename, READONLY);
                // See if the file actually opened
                if (thisVector[thisSide][thisHalf][thisDimension].inputFile == NULL)
                {
                    // If it didn't...
                    fprintf(stderr, MESSAGE_ERROR);
                    fprintf(stderr, MESSAGE_FILE_OPENERROR, filename);                              
                    // Exit
                    exit(EXIT_FAILURE);
                }
                // Read the first value of the file: The total number of
                // point values for this vector
                fscanf(thisVector[thisSide][thisHalf][thisDimension].inputFile, "%d",
                   &thisVector[thisSide][thisHalf][thisDimension].totalValues);
                // See if the value a number greater than zero
                if (thisVector[thisSide][thisHalf][thisDimension].totalValues <= 0)
                {
                    // If it's not...
                    fprintf(stderr, MESSAGE_ERROR);
                    fprintf(stderr, MESSAGE_FILE_READERROR, filename);
                    // Exit
                    exit(EXIT_FAILURE);
                }
                // Free the memory used by the filename variable
                free(filename);
            }
        }
    }
}
        

// Function name: CloseDataFiles()
// Purpose: Closes all input and output files required by the program.
//
void CloseDataFiles()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;
        
    // Close the output file
    fclose(outputFile);
        
    // Iterate through all vectors...
    for (thisSide = Root; thisSide <= Tip; thisSide++)
    {
        for (thisHalf = Upper; thisHalf <= Lower; thisHalf++)
        {        
            for (thisDimension = X; thisDimension <= Y; thisDimension++)
            {                       
                // Close the input file
                fclose(thisVector[thisSide][thisHalf][thisDimension].inputFile);        
            }
        }
    }
}


// Function name: AllocateMemory()
// Purpose: Requests RAM from the operating system. This memory is then used
//          to store the all of the vector data points.
//
void AllocateMemory()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;
        
    // Iterate through all vectors...
    for (thisSide = Root; thisSide <= Tip; thisSide++)
    {
        for (thisHalf = Upper; thisHalf <= Lower; thisHalf++)
        {        
            for (thisDimension = X; thisDimension <= Y; thisDimension++)
            {
                // Allocate memory based on the total number of data values
                thisVector[thisSide][thisHalf][thisDimension].value = 
                  (float *)malloc(thisVector[thisSide][thisHalf][thisDimension].totalValues *
                  sizeof(float));
                // See if the memory allocation is successful
                if (thisVector[thisSide][thisHalf][thisDimension].value == NULL)
                {
                    // If it's not...
                    fprintf(stderr, MESSAGE_ERROR);
                    fprintf(stderr, MESSAGE_MEMORY_ALLOCERROR);
                    // Exit
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}


// Function name: FreeMemory()
// Purpose: Releases all allocated vector data point memory back to
//          the operating system.
//
void FreeMemory()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;
        
    // Iterate through all vectors...
    for (thisSide = Root; thisSide <= Tip; thisSide++)
    {
        for (thisHalf = Upper; thisHalf <= Lower; thisHalf++)
        {       
            for (thisDimension = X; thisDimension <= Y; thisDimension++)
            {
                // Free allocated memory
                free(thisVector[thisSide][thisHalf][thisDimension].value);
            }
        }
    }
}


// Function name: CheckVectorConsistency()
// Purpose: Performs a quick check to see if all vectors advertise
//          the same number of data points. Displays a warning if not.
//
void CheckVectorConsistency()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;
        
    int reference;
    
    // Select an arbitrary reference
    reference = 
     thisVector[thisSide = Tip][thisHalf = Lower][thisDimension = Y].totalValues;
        
    // Check every other vector against that reference
    if (!((thisVector[thisSide = Root][thisHalf = Upper][thisDimension = X].totalValues == reference)
     && (thisVector[thisSide = Root][thisHalf = Upper][thisDimension = Y].totalValues == reference)
     && (thisVector[thisSide = Root][thisHalf = Lower][thisDimension = X].totalValues == reference)
     && (thisVector[thisSide = Root][thisHalf = Lower][thisDimension = Y].totalValues == reference)
     && (thisVector[thisSide = Tip][thisHalf = Upper][thisDimension = X].totalValues == reference)
     && (thisVector[thisSide = Tip][thisHalf = Upper][thisDimension = Y].totalValues == reference)
     && (thisVector[thisSide = Tip][thisHalf = Lower][thisDimension = X].totalValues == reference)))
    {
        // If they aren't consistent...
        fprintf(stderr, MESSAGE_WARNING);
        fprintf(stderr, MESSAGE_VECTOR_CONSISTENCY);
    }
}


// Function name: ReadVectorData()
// Purpose: Reads all vector data points from their files and
//          into allocated memory.
//
void ReadVectorData()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;
        
    int thisValue;
    int result;
        
    // Iterate through all vectors...
    for (thisSide = Root; thisSide <= Tip; thisSide++)
    {
        for (thisHalf = Upper; thisHalf <= Lower; thisHalf++)
        {  
            for (thisDimension = X; thisDimension <= Y; thisDimension++)
            {                       
                // Read all data values into allocated memory
                for (thisValue = 0;
                     thisValue < thisVector[thisSide][thisHalf][thisDimension].totalValues;
                     thisValue++)
                {
                    // Read the value from the proper input file. The success
                    // or failure of the read operation is stored in the "result"
                    // variable.
                    result = fscanf(thisVector[thisSide][thisHalf][thisDimension].inputFile, "%f",
                             &thisVector[thisSide][thisHalf][thisDimension].value[thisValue]);
                    // Scale the value by the coordinate scalar
                    thisVector[thisSide][thisHalf][thisDimension].value[thisValue] *= XYUV_COORDINATE_SCALAR;
                    // See if End-of-File was reached unexpectedly
                    if (result == EOF)
                    {
                        // If it was...
                        fprintf(stderr, MESSAGE_WARNING);
                        fprintf(stderr, MESSAGE_VECTOR_EOF);
                        // Just abort reading this vector; don't exit the program.
                        break;
                    }
                }
            }
        }
    }
}


// Function name: OutputGCode()
// Purpose: Produces valid GCode from the raw data points and writes 
//          it to an output file. This function does not buffer writes to be
//          "actually written" at a later time -- All writes are performed
//          in real-time.
//
void OutputGCode()
{
    enum Side thisSide;
    enum Half thisHalf;
    enum Dimension thisDimension;

    int thisValue;
    int result;


    // Output the GCode header ////////////////////////////////////////////////
    fprintf(outputFile, GCODE_HEADER);
    ///////////////////////////////////////////////////////////////////////////

        
    // Iterate through all of the data points for the Upper airfoil half, using
    // TIPUPPERX as the reference for the total number of data points.
    thisHalf = Upper;
    for (thisValue = 0;
         thisValue < thisVector[thisSide = Tip][thisHalf][thisDimension = X].totalValues;
         thisValue++)
    {
        // Output one line of airfoil coordinates /////////////////////////////
        fprintf(outputFile, "%s %s %s%f %s%f ",
                GCODE_MOVE_COMMAND,
                GCODE_FEEDRATE,
                DimensionToString[thisDimension = X],
                thisVector[thisSide = Root][thisHalf][thisDimension = X].value[thisValue],
                DimensionToString[thisDimension = Y],
                thisVector[thisSide = Root][thisHalf][thisDimension = Y].value[thisValue]);
        fprintf(outputFile, "U%f V%f\n", 
                thisVector[thisSide = Tip][thisHalf][thisDimension = X].value[thisValue],
                thisVector[thisSide = Tip][thisHalf][thisDimension = Y].value[thisValue]);
        ///////////////////////////////////////////////////////////////////////
    }
        
        
    // Output the transition between the Upper and Lower halves ///////////////
    fprintf(outputFile, GCODE_UPPERLOWER_TRANSITION);
    ///////////////////////////////////////////////////////////////////////////
        
        
    // Iterate through all of the data points for the Lower airfoil half, using
    // TIPLOWERX as the reference for the total number of data points.
    thisHalf = Lower;
    for (thisValue = 0;
        thisValue < thisVector[thisSide = Tip][thisHalf][thisDimension = X].totalValues;
         thisValue++)
    {
        // Output one line of airfoil coordinates /////////////////////////////
        fprintf(outputFile, "%s %s %s%f %s%f ",
                GCODE_MOVE_COMMAND,
                GCODE_FEEDRATE,
                DimensionToString[thisDimension = X],
                thisVector[thisSide = Root][thisHalf][thisDimension = X].value[thisValue],
                DimensionToString[thisDimension = Y],
                thisVector[thisSide = Root][thisHalf][thisDimension = Y].value[thisValue]);
        fprintf(outputFile, "U%f V%f\n", 
                thisVector[thisSide = Tip][thisHalf][thisDimension = X].value[thisValue],
                thisVector[thisSide = Tip][thisHalf][thisDimension = Y].value[thisValue]);
        ///////////////////////////////////////////////////////////////////////
    }   

        
    // Output the GCode footer ////////////////////////////////////////////////
    result = fprintf(outputFile, GCODE_FOOTER);
    ///////////////////////////////////////////////////////////////////////////
    
       
    // See if the final write to the output file failed
    if (result < 0)
    {
        // If it did...
        fprintf(stderr, MESSAGE_ERROR);
        fprintf(stderr, MESSAGE_FILE_WRITEERROR, OUTPUT_FILENAME);
        // Exit
        exit(EXIT_FAILURE);
    }
}


// --- End of gcode.c
