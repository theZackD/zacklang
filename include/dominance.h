#ifndef DOMINANCE_H
#define DOMINANCE_H

#include "zir.h"
#include <stdbool.h>

// Structure to hold dominance information for a block
typedef struct DominanceInfo
{
    ZIRBlock *block;               // The block this info belongs to
    ZIRBlock **dominators;         // Array of blocks that dominate this block
    int dominator_count;           // Number of dominators
    int dominator_capacity;        // Capacity of dominators array
    ZIRBlock *immediate_dominator; // Immediate dominator of this block
    ZIRBlock **frontier;           // Dominance frontier
    int frontier_count;            // Number of blocks in dominance frontier
    int frontier_capacity;         // Capacity of frontier array
} DominanceInfo;

// Structure to hold dominance information for a function
typedef struct DominanceAnalysis
{
    ZIRFunction *function;      // The function being analyzed
    DominanceInfo **block_info; // Array of dominance info for each block
    int block_count;            // Number of blocks
} DominanceAnalysis;

// Create and destroy dominance analysis
DominanceAnalysis *create_dominance_analysis(ZIRFunction *function);
void destroy_dominance_analysis(DominanceAnalysis *analysis);

// Core analysis functions
void compute_dominators(DominanceAnalysis *analysis);
void compute_dominance_frontier(DominanceAnalysis *analysis);

// Helper functions
bool block_dominates(DominanceAnalysis *analysis, ZIRBlock *dominator, ZIRBlock *block);
ZIRBlock *get_immediate_dominator(DominanceAnalysis *analysis, ZIRBlock *block);
ZIRBlock **get_dominance_frontier(DominanceAnalysis *analysis, ZIRBlock *block, int *count);

// Debug functions
void print_dominance_info(DominanceAnalysis *analysis);
void print_dominance_frontier(DominanceAnalysis *analysis, ZIRBlock *block);

#endif // DOMINANCE_H