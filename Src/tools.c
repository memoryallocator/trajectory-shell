#include "tools.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

const bool kDbgTools = false;

struct dfa_node_mutable {
	struct dfa_node_mutable *next[128];
	size_t autocompletion_offset;
	size_t autocompletion_variants_count;
	char **autocompletion_variants;
};

struct dfa_mutable {
	struct dfa_node_mutable root;
};

static void dfa_get_matching_node_subroutine(const struct dfa_node *curr_node, const char *const s,
																						 struct dfa_node **const res) {
	for (size_t i = 0; s[i] != '\0'; ++i) {
		const char kCurrSymbol = s[i];
		const size_t kCurrSymbolAsciiValue = (size_t)kCurrSymbol;
		const struct dfa_node *kNextNode = curr_node->next[kCurrSymbolAsciiValue];
		if (kNextNode == NULL) {
			*res = NULL;
			return;
		}
		curr_node = kNextNode;
	}

	*res = (struct dfa_node *)curr_node;  // found
}

static const char dfa_is_null[] = "dfa is NULL";
static const char string_is_null[] = "string is NULL";
static const char no_autocompletion_variants_available[] = "no autocompletion variants available";
static const char smth_went_wrong[] = "smth went wrong";

const char *dfa_get_next_autocompletion_variant(const struct dfa *const dfa, const char *const s) {
	if (dfa == NULL or s == NULL) {

		if (kDbgTools) {
			if (dfa == NULL) {
				return dfa_is_null;
			}
			return string_is_null;
		}

		return NULL;
	}
	struct dfa_node *matching_node = malloc(sizeof(struct dfa_node));
	if (matching_node == NULL) {
		abort();
	}
	dfa_get_matching_node_subroutine(&dfa->root, s, &matching_node);
	if (matching_node == NULL) {  // failed to find a match

		if (kDbgTools) {
			char *const reason = calloc(101, sizeof(char));
			if (dfa->root.next[(size_t)'l'] == NULL) {
				sprintf(reason, "NULL");
			} else {
				sprintf(reason,
								"node %zu has %zu autocompletion variants",
								dfa->root.next[(size_t)'l'],
								dfa->root.next[(size_t)'l']->autocompletion_variants_count);
			}

			char *const res = calloc(101, sizeof(char));
			sprintf(res, "failed to find a match, %s", reason);
			free(reason);
			return res;
		}

		return NULL;
	}

	if (matching_node->autocompletion_variants_count == 0) {

		if (kDbgTools) {
			return no_autocompletion_variants_available;
		}

		return NULL;
	}

	const char *const res = matching_node->autocompletion_variants[matching_node->autocompletion_offset];
	matching_node->autocompletion_offset++;
	matching_node->autocompletion_offset %= matching_node->autocompletion_variants_count;

	if (kDbgTools) {
		if (res == NULL) {
			return smth_went_wrong;
		}
	}

	return res;
}

static struct dfa_node_mutable *dfa_node_mutable_create_empty() {
	struct dfa_node_mutable *new_node = malloc(sizeof(struct dfa_node_mutable));
	if (new_node == NULL) {
		abort();
	}
	*new_node = (struct dfa_node_mutable){ 0 };
	return new_node;
}

static struct dfa_node_mutable *
dfa_node_mutable_append(struct dfa_node_mutable **const node, const char *const command) {
	if (*node == NULL) {
		abort();
	}

	(*node)->autocompletion_variants = realloc((*node)->autocompletion_variants,
																						 sizeof(char *) * ((*node)->autocompletion_variants_count + 1));
	(*node)->autocompletion_variants[(*node)->autocompletion_variants_count] = calloc(strlen(command) + 1, sizeof(char));
	strcpy((*node)->autocompletion_variants[(*node)->autocompletion_variants_count], command);
	(*node)->autocompletion_variants_count++;
	return *node;
}

static struct dfa_node_mutable *dfa_node_mutable_create(const char *const command) {
	struct dfa_node_mutable *empty_node = dfa_node_mutable_create_empty();
	return dfa_node_mutable_append(&empty_node, command);
}

static struct dfa_node_mutable *
dfa_node_mutable_append_or_create(struct dfa_node_mutable **const node, const char *const command) {
	if (*node == NULL) {
		*node = dfa_node_mutable_create(command);
		return *node;
	}

	return dfa_node_mutable_append(node, command);
}

static struct dfa_node_mutable *
dfa_insert_everything_but_last_symbol_of_token(struct dfa_node_mutable *last_matching_node_ptr,
																							 const char *const token) {
	for (size_t j = 0; token[j + 1] != '\0'; ++j) {
		last_matching_node_ptr = dfa_node_mutable_append_or_create(&(last_matching_node_ptr->next[(size_t)token[j]]),
																															 token + j + 1);
	}
	return last_matching_node_ptr;
}

static void dfa_run_callback_on_each_sibling(const struct dfa_node *const node,
																						 void (*callback_function)(const struct dfa_node *const)) {
	for (size_t i = 0; i < 128; ++i) {
		const struct dfa_node *const next_node = node->next[i];
		if (next_node != NULL) {
			dfa_run_callback_on_each_sibling(next_node, callback_function);
			callback_function(next_node);
		}
	}
}

static int cmp_str(const void *a, const void *b) {
	return strcmp((const char *)a, (const char *)b);
}

static void dfa_make_autocompletion_variants_unique(const struct dfa_node *const node) {
	if (node->autocompletion_variants_count == 0) {
		return;
	}

	struct dfa_node_mutable *const mut_node = (struct dfa_node_mutable *)node;

	qsort(mut_node->autocompletion_variants, mut_node->autocompletion_variants_count, sizeof(char *), cmp_str);
	size_t unique_count = 1;
	size_t *unique_ids = calloc(1, sizeof(size_t));
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

void dfa_node_reset(const struct dfa_node *const node) {
	((struct dfa_node *const)node)->autocompletion_offset = 0;
}

struct dfa *autocompletion_dfa_reset(struct dfa *const dfa) {
	dfa_run_callback_on_each_sibling(&(dfa->root), dfa_node_reset);
	return dfa;
}

struct dfa autocompletion_dfa_create(char *const *const commands, const size_t commands_count) {
	struct dfa_mutable res = { 0 };
	for (size_t i = 0; i < commands_count; ++i) {
		struct dfa_node_mutable *last_matching_node = &res.root;
		char *token = strtok(commands[i], " ");

		last_matching_node = dfa_insert_everything_but_last_symbol_of_token(last_matching_node, token);

		while (true) {
			const size_t kPrevTokenNameLen = strlen(token);
			char *prev_token = calloc(strlen(token) + 1, sizeof(char));
			if (prev_token == NULL) {
				abort();
			}

			strcpy(prev_token, token);
			token = strtok(NULL, " ");
			if (token == NULL) {
				break;
			}

			char *space_plus_token = calloc(strlen(token) + 2, sizeof(char));
			if (space_plus_token == NULL) {
				abort();
			}
			space_plus_token[0] = ' ';
			strcpy(space_plus_token + 1, token);

			struct dfa_node_mutable **const kLastSymbolOfPrevWordNodePtr = &(last_matching_node->next[
					(size_t)prev_token[kPrevTokenNameLen - 1]]);
			last_matching_node = dfa_node_mutable_append_or_create(kLastSymbolOfPrevWordNodePtr, space_plus_token);

			last_matching_node = dfa_node_mutable_append_or_create(&(last_matching_node->next[(size_t)' ']), token);
			last_matching_node = dfa_insert_everything_but_last_symbol_of_token(last_matching_node, token);
		}
	}

	dfa_run_callback_on_each_sibling((const struct dfa_node *const)&res.root, dfa_make_autocompletion_variants_unique);
	return *((struct dfa *)&res);
}