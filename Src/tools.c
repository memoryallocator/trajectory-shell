#include "tools.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef DBG_TOOLS

#include <stdio.h>

#endif

struct dfa_node_mutable_t {
    struct dfa_node_mutable_t* next[128];
    size_t autocompletion_offset;
    size_t autocompletion_variants_count;
    char** autocompletion_variants;
};

struct dfa_mutable_t {
    struct dfa_node_mutable_t root;
};

static void dfa_get_matching_node_subroutine(const struct dfa_node_t* curr_node, const char* const s,
                                             struct dfa_node_t** const res) {
  const char first_symbol = s[0];
  if (first_symbol == '\0') {
    *res = (struct dfa_node_t*) curr_node;
    return;
  }
  const size_t first_symbol_ascii_value = (size_t) first_symbol;
  const struct dfa_node_t* kNextNode = curr_node->next[first_symbol_ascii_value];
  if (kNextNode == NULL) {
    *res = NULL;
    return;
  }
  dfa_get_matching_node_subroutine(kNextNode, s + 1, res);
}

#ifdef DBG_TOOLS
static const char* dfa_is_null = "dfa is NULL";
static const char* string_is_null = "string is NULL";
static const char* no_autocompletion_variants_available = "no autocompletion variants available";
static const char* smth_went_wrong = "smth went wrong";
#endif

const char* dfa_get_next_autocompletion_variant(const struct dfa_t* const dfa, const char* const s) {
  if (dfa == NULL or s == NULL) {
#ifdef DBG_TOOLS
    if (dfa == NULL) {
      return dfa_is_null;
    }
    return string_is_null;
#endif
    return NULL;
  }
  struct dfa_node_t* matching_node = malloc(sizeof(struct dfa_node_t));
  if (matching_node == NULL) {
    abort();
  }
  dfa_get_matching_node_subroutine(&dfa->root, s, &matching_node);
  if (matching_node == NULL) {  // failed to find a match
#ifdef DBG_TOOLS
    char *const reason = calloc(101, sizeof(char));
    if (dfa->root.next[(size_t) 'l'] == NULL) {
      sprintf(reason, "NULL");
    } else {
      sprintf(reason, "node %d has %d autocompletion variants",          dfa->root.next[(size_t) 'l'],          dfa->root.next[(size_t) 'l']->autocompletion_variants_count);
    }

    char *const res = calloc(101, sizeof(char));
    sprintf(res, "failed to find a match, %s", reason);
    free(reason);
    return res;
#endif
    return NULL;
  }

  if (matching_node->autocompletion_variants_count == 0) {
#ifdef DBG_TOOLS
    return no_autocompletion_variants_available;
#endif
    return NULL;
  }

  const char* const res = matching_node->autocompletion_variants[matching_node->autocompletion_offset];
  matching_node->autocompletion_offset++;
  matching_node->autocompletion_offset %= matching_node->autocompletion_variants_count;
#ifdef DBG_TOOLS
  if (res == NULL) {
    return smth_went_wrong;
  }
#endif
  return res;
}

static struct dfa_node_mutable_t* dfa_node_mutable_create_empty() {
  struct dfa_node_mutable_t* new_node = malloc(sizeof(struct dfa_node_mutable_t));
  if (new_node == NULL) {
    abort();
  }
  *new_node = (struct dfa_node_mutable_t) { 0 };
  return new_node;
}

static struct dfa_node_mutable_t*
dfa_node_mutable_append(struct dfa_node_mutable_t** const node, const char* const command) {
  if (*node == NULL) {
    abort();
  }

  (*node)->autocompletion_variants = realloc((*node)->autocompletion_variants,
                                             sizeof(char*) * ((*node)->autocompletion_variants_count + 1));
  (*node)->autocompletion_variants[(*node)->autocompletion_variants_count] = calloc(strlen(command) + 1, sizeof(char));
  strcpy((*node)->autocompletion_variants[(*node)->autocompletion_variants_count], command);
  (*node)->autocompletion_variants_count++;
  return *node;
}

static struct dfa_node_mutable_t* dfa_node_mutable_create(const char* const command) {
  struct dfa_node_mutable_t* empty_node = dfa_node_mutable_create_empty();
  return dfa_node_mutable_append(&empty_node, command);
}

static struct dfa_node_mutable_t*
dfa_node_mutable_append_or_create(struct dfa_node_mutable_t** const node, const char* const command) {
  if (*node == NULL) {
    *node = dfa_node_mutable_create(command);
    return *node;
  }

  return dfa_node_mutable_append(node, command);
}

static struct dfa_node_mutable_t*
dfa_insert_everything_but_last_symbol_of_token(struct dfa_node_mutable_t* last_matching_node_ptr,
                                               const char* const token) {
  for (size_t j = 0; token[j + 1] != '\0'; ++j) {
    last_matching_node_ptr = dfa_node_mutable_append_or_create(&(last_matching_node_ptr->next[(size_t) token[j]]),
                                                               token + j + 1);
  }
  return last_matching_node_ptr;
}

static void dfa_run_callback_on_each_sibling(const struct dfa_node_t* const node,
                                             void (* callback_function)(const struct dfa_node_t* const)) {
  for (size_t i = 0; i < 128; ++i) {
    const struct dfa_node_t* const next_node = node->next[i];
    if (next_node != NULL) {
      dfa_run_callback_on_each_sibling(next_node, callback_function);
      callback_function(next_node);
    }
  }
}

static int cmp_str(const void* a, const void* b) {
  const char** const a_as_str_ptr = (const char**) a;
  const char** const b_as_str_ptr = (const char**) b;

  return strcmp(*a_as_str_ptr, *b_as_str_ptr);
}

static void dfa_make_autocompletion_variants_unique(const struct dfa_node_t* const node) {
  if (node->autocompletion_variants_count == 0) {
    return;
  }

  struct dfa_node_mutable_t* const mut_node = (struct dfa_node_mutable_t*) node;

  qsort(mut_node->autocompletion_variants, mut_node->autocompletion_variants_count, sizeof(char*), cmp_str);
  size_t unique_count = 1;
  size_t* unique_ids = calloc(1, sizeof(size_t));
  for (size_t i = 1; i < mut_node->autocompletion_variants_count; ++i) {
    if (strcmp(mut_node->autocompletion_variants[i - 1], mut_node->autocompletion_variants[i]) != 0) {
      ++unique_count;
      unique_ids = realloc(unique_ids, sizeof(size_t) * unique_count);
      unique_ids[unique_count - 1] = i;
    }
  }

  for (size_t i = 1; i < unique_count; ++i) {
    strcpy(mut_node->autocompletion_variants[i], mut_node->autocompletion_variants[unique_ids[i]]);
  }
  mut_node->autocompletion_variants = realloc(mut_node->autocompletion_variants, sizeof(size_t) * unique_count);
  mut_node->autocompletion_variants_count = unique_count;
}

void dfa_node_reset(const struct dfa_node_t* const node) {
  ((struct dfa_node_t* const) node)->autocompletion_offset = 0;
}

struct dfa_t* autocompletion_dfa_reset(struct dfa_t* const dfa) {
  dfa_run_callback_on_each_sibling(&(dfa->root), dfa_node_reset);
  return dfa;
}

struct dfa_t autocompletion_dfa_create(char* const* const commands, const size_t commands_count) {
  struct dfa_mutable_t res = { 0 };
  for (size_t i = 0; i < commands_count; ++i) {
    struct dfa_node_mutable_t* last_matching_node = &res.root;
    char* token = strtok(commands[i], " ");

    last_matching_node = dfa_insert_everything_but_last_symbol_of_token(last_matching_node, token);

    while (true) {
      const size_t kPrevTokenNameLen = strlen(token);
      char* prev_token = calloc(strlen(token) + 1, sizeof(char));
      strcpy(prev_token, token);
      token = strtok(NULL, " ");
      if (token == NULL) {
        break;
      }

      char* space_plus_token = calloc(strlen(token) + 2, sizeof(char));
      if (space_plus_token == NULL) {
        abort();
      }
      space_plus_token[0] = ' ';
      strcpy(space_plus_token + 1, token);

      struct dfa_node_mutable_t** const last_symbol_of_prev_word_node_ptr = &(last_matching_node->next[
        (size_t) prev_token[kPrevTokenNameLen - 1]]);
      last_matching_node = dfa_node_mutable_append_or_create(last_symbol_of_prev_word_node_ptr, space_plus_token);

      last_matching_node = dfa_node_mutable_append_or_create(&(last_matching_node->next[(size_t) ' ']), token);
      last_matching_node = dfa_insert_everything_but_last_symbol_of_token(last_matching_node, token);
    }
  }

  dfa_run_callback_on_each_sibling((const struct dfa_node_t* const) &res.root, dfa_make_autocompletion_variants_unique);
  return *((struct dfa_t*) &res);
}