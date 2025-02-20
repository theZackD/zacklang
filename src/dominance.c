#include "include/dominance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

// Helper function for memory allocation
static void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}

// Helper function to create dominance info for a block
static DominanceInfo *create_dominance_info(ZIRBlock *block)
{
    DominanceInfo *info = safe_malloc(sizeof(DominanceInfo));
    info->block = block;
    info->dominator_capacity = INITIAL_CAPACITY;
    info->dominators = safe_malloc(info->dominator_capacity * sizeof(ZIRBlock *));
    info->dominator_count = 0;
    info->immediate_dominator = NULL;
    info->frontier_capacity = INITIAL_CAPACITY;
    info->frontier = safe_malloc(info->frontier_capacity * sizeof(ZIRBlock *));
    info->frontier_count = 0;
    return info;
}

// Create dominance analysis
DominanceAnalysis *create_dominance_analysis(ZIRFunction *function)
{
    if (!function)
        return NULL;

    DominanceAnalysis *analysis = safe_malloc(sizeof(DominanceAnalysis));
    analysis->function = function;
    analysis->block_count = function->block_count;
    analysis->block_info = safe_malloc(analysis->block_count * sizeof(DominanceInfo *));

    // Create dominance info for each block
    for (int i = 0; i < analysis->block_count; i++)
    {
        analysis->block_info[i] = create_dominance_info(function->blocks[i]);
    }

    return analysis;
}

// Helper to get index of a block
static int get_block_index(DominanceAnalysis *analysis, ZIRBlock *block)
{
    for (int i = 0; i < analysis->block_count; i++)
    {
        if (analysis->block_info[i]->block == block)
        {
            return i;
        }
    }
    return -1;
}

// Helper to add a dominator
static void add_dominator(DominanceInfo *info, ZIRBlock *dominator)
{
    // Check if dominator already exists
    for (int i = 0; i < info->dominator_count; i++)
    {
        if (info->dominators[i] == dominator)
            return;
    }

    // Resize if needed
    if (info->dominator_count >= info->dominator_capacity)
    {
        info->dominator_capacity *= 2;
        info->dominators = realloc(info->dominators,
                                   info->dominator_capacity * sizeof(ZIRBlock *));
        if (!info->dominators)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    info->dominators[info->dominator_count++] = dominator;
}

// Helper to add a block to frontier
static void add_to_frontier(DominanceInfo *info, ZIRBlock *block)
{
    // Check if block already exists in frontier
    for (int i = 0; i < info->frontier_count; i++)
    {
        if (info->frontier[i] == block)
            return;
    }

    // Resize if needed
    if (info->frontier_count >= info->frontier_capacity)
    {
        info->frontier_capacity *= 2;
        info->frontier = realloc(info->frontier,
                                 info->frontier_capacity * sizeof(ZIRBlock *));
        if (!info->frontier)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    info->frontier[info->frontier_count++] = block;
}

// Helper to compute intersection of dominator sets
static void intersect_dominators(DominanceInfo *target, DominanceInfo *source)
{
    int i = 0;
    while (i < target->dominator_count)
    {
        bool found = false;
        for (int j = 0; j < source->dominator_count; j++)
        {
            if (target->dominators[i] == source->dominators[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            // Remove this dominator
            for (int k = i; k < target->dominator_count - 1; k++)
            {
                target->dominators[k] = target->dominators[k + 1];
            }
            target->dominator_count--;
        }
        else
        {
            i++;
        }
    }
}

// Helper to clear dominators
static void clear_dominators(DominanceInfo *info)
{
    info->dominator_count = 0;
}

// Compute dominators using iterative algorithm
void compute_dominators(DominanceAnalysis *analysis)
{
    if (!analysis || analysis->block_count == 0)
        return;

    // Initialize entry block to dominate itself
    DominanceInfo *entry_info = analysis->block_info[0];
    clear_dominators(entry_info);
    add_dominator(entry_info, entry_info->block);

    // Initialize all other blocks to be dominated by all blocks
    for (int i = 1; i < analysis->block_count; i++)
    {
        DominanceInfo *info = analysis->block_info[i];
        clear_dominators(info);
        for (int j = 0; j < analysis->block_count; j++)
        {
            add_dominator(info, analysis->block_info[j]->block);
        }
    }

    bool changed;
    int iterations = 0;
    const int MAX_ITERATIONS = 1000; // Safety limit

    do
    {
        changed = false;
        // For each block except entry
        for (int i = 1; i < analysis->block_count; i++)
        {
            DominanceInfo *info = analysis->block_info[i];
            ZIRBlock *block = info->block;

            if (block->pred_count > 0)
            {
                // Get first predecessor's dominators
                ZIRBlock *first_pred = block->predecessors[0];
                int pred_idx = get_block_index(analysis, first_pred);
                if (pred_idx < 0)
                    continue;

                DominanceInfo *pred_info = analysis->block_info[pred_idx];

                // Start with first predecessor's dominators
                int old_count = info->dominator_count;
                clear_dominators(info);
                for (int j = 0; j < pred_info->dominator_count; j++)
                {
                    add_dominator(info, pred_info->dominators[j]);
                }

                // Intersect with other predecessors
                for (int p = 1; p < block->pred_count; p++)
                {
                    ZIRBlock *pred = block->predecessors[p];
                    pred_idx = get_block_index(analysis, pred);
                    if (pred_idx < 0)
                        continue;

                    pred_info = analysis->block_info[pred_idx];
                    intersect_dominators(info, pred_info);
                }

                // Add this block to its own dominators
                add_dominator(info, block);

                // Check if anything changed
                if (old_count != info->dominator_count)
                {
                    changed = true;
                }
            }
        }

        if (++iterations >= MAX_ITERATIONS)
        {
            fprintf(stderr, "Warning: Dominance computation exceeded iteration limit\n");
            break;
        }
    } while (changed);

    // Compute immediate dominators
    for (int i = 1; i < analysis->block_count; i++)
    {
        DominanceInfo *info = analysis->block_info[i];

        // Find the dominator that is dominated by all other dominators
        ZIRBlock *idom = NULL;
        for (int j = 0; j < info->dominator_count; j++)
        {
            ZIRBlock *dom = info->dominators[j];
            if (dom == info->block)
                continue;

            bool is_idom = true;
            for (int k = 0; k < info->dominator_count; k++)
            {
                ZIRBlock *other_dom = info->dominators[k];
                if (other_dom == dom || other_dom == info->block)
                    continue;

                if (!block_dominates(analysis, other_dom, dom))
                {
                    is_idom = false;
                    break;
                }
            }
            if (is_idom)
            {
                idom = dom;
                break;
            }
        }
        info->immediate_dominator = idom;
    }
}

// Compute dominance frontier
void compute_dominance_frontier(DominanceAnalysis *analysis)
{
    if (!analysis)
        return;

    // For each block
    for (int i = 0; i < analysis->block_count; i++)
    {
        DominanceInfo *info = analysis->block_info[i];
        ZIRBlock *block = info->block;

        // Clear existing frontier
        info->frontier_count = 0;

        // For each successor of the block
        for (int s = 0; s < block->succ_count; s++)
        {
            ZIRBlock *succ = block->successors[s];
            int succ_idx = get_block_index(analysis, succ);
            if (succ_idx < 0)
                continue;

            // If this block doesn't dominate its successor, the successor is in its frontier
            if (!block_dominates(analysis, block, succ) || block == succ)
            {
                add_to_frontier(info, succ);
            }
        }

        // For each predecessor of the block
        for (int p = 0; p < block->pred_count; p++)
        {
            ZIRBlock *pred = block->predecessors[p];
            int pred_idx = get_block_index(analysis, pred);
            if (pred_idx < 0)
                continue;

            // Walk up the dominator tree from the predecessor
            ZIRBlock *runner = pred;
            int runner_idx = get_block_index(analysis, runner);

            while (runner && runner != info->immediate_dominator)
            {
                if (runner_idx >= 0)
                {
                    add_to_frontier(analysis->block_info[runner_idx], block);
                    runner = analysis->block_info[runner_idx]->immediate_dominator;
                    if (runner)
                    {
                        runner_idx = get_block_index(analysis, runner);
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
}

// Check if one block dominates another
bool block_dominates(DominanceAnalysis *analysis, ZIRBlock *dominator, ZIRBlock *block)
{
    if (!analysis || !dominator || !block)
        return false;

    int block_idx = get_block_index(analysis, block);
    if (block_idx < 0)
        return false;

    DominanceInfo *info = analysis->block_info[block_idx];
    for (int i = 0; i < info->dominator_count; i++)
    {
        if (info->dominators[i] == dominator)
            return true;
    }
    return false;
}

// Get immediate dominator of a block
ZIRBlock *get_immediate_dominator(DominanceAnalysis *analysis, ZIRBlock *block)
{
    if (!analysis || !block)
        return NULL;

    int block_idx = get_block_index(analysis, block);
    if (block_idx < 0)
        return NULL;

    return analysis->block_info[block_idx]->immediate_dominator;
}

// Get dominance frontier of a block
ZIRBlock **get_dominance_frontier(DominanceAnalysis *analysis, ZIRBlock *block, int *count)
{
    if (!analysis || !block || !count)
        return NULL;

    int block_idx = get_block_index(analysis, block);
    if (block_idx < 0)
        return NULL;

    DominanceInfo *info = analysis->block_info[block_idx];
    *count = info->frontier_count;
    return info->frontier;
}

// Print dominance info for debugging
void print_dominance_info(DominanceAnalysis *analysis)
{
    if (!analysis)
        return;

    printf("Dominance Analysis:\n");
    for (int i = 0; i < analysis->block_count; i++)
    {
        DominanceInfo *info = analysis->block_info[i];
        printf("Block %s:\n", info->block->label);

        printf("  Dominators: ");
        for (int j = 0; j < info->dominator_count; j++)
        {
            printf("%s ", info->dominators[j]->label);
        }
        printf("\n");

        if (info->immediate_dominator)
        {
            printf("  Immediate Dominator: %s\n",
                   info->immediate_dominator->label);
        }

        printf("  Dominance Frontier: ");
        for (int j = 0; j < info->frontier_count; j++)
        {
            printf("%s ", info->frontier[j]->label);
        }
        printf("\n\n");
    }
}

// Print dominance frontier for a specific block
void print_dominance_frontier(DominanceAnalysis *analysis, ZIRBlock *block)
{
    if (!analysis || !block)
        return;

    int block_idx = get_block_index(analysis, block);
    if (block_idx < 0)
        return;

    DominanceInfo *info = analysis->block_info[block_idx];
    printf("Dominance Frontier for block %s: ", block->label);
    for (int i = 0; i < info->frontier_count; i++)
    {
        printf("%s ", info->frontier[i]->label);
    }
    printf("\n");
}

// Clean up
void destroy_dominance_analysis(DominanceAnalysis *analysis)
{
    if (!analysis)
        return;

    for (int i = 0; i < analysis->block_count; i++)
    {
        DominanceInfo *info = analysis->block_info[i];
        free(info->dominators);
        free(info->frontier);
        free(info);
    }
    free(analysis->block_info);
    free(analysis);
}