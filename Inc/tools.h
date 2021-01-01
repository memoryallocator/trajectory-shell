#ifndef TRAJECTORY_SHELL_TOOLS_H
#define TRAJECTORY_SHELL_TOOLS_H

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>

// Deterministic Finite Automata (ASCII-only) node
struct dfa_node_t {
    const struct dfa_node_t* const next[128];
    size_t autocompletion_offset;
    const size_t autocompletion_variants_count;
    const char** const autocompletion_variants;
};

// Deterministic Finite Automata (ASCII-only)
struct dfa_t {
    struct dfa_node_t root;
};

const char* dfa_get_next_autocompletion_variant(const struct dfa_t*, const char*);

struct dfa_t* autocompletion_dfa_reset(struct dfa_t* dfa);

struct dfa_t autocompletion_dfa_create(char* const* commands, size_t commands_count);

#endif  // TRAJECTORY_SHELL_TOOLS_H
