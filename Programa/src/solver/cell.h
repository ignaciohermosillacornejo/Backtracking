#pragma once

typedef struct cell Cell;
/** A simple struct to hold some properties */
struct cell
{
    int row;
    int col;
    int degree;  // From 0 to 4
    int status;  // True for rebel, False if not
    int color;   // To keep track of visited nodes
    int rebels;  // Number of rebel neighbours
    int loyals;  // Number of loyal neighbours
    int empty;   // Number of empty neighbours
    int priority; // Number of ways to put the cell, can be 1 or 2
};
