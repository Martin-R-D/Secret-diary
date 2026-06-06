#ifndef SEARCH_H
#define SEARCH_H

typedef struct {
    char title[100];
    char date[11]; // YYYY-MM-DD
    char body[1024];
} DiaryEntry;

// Load all entries from a binary file into a dynamically allocated array.
// Returns the number of entries loaded, or -1 on error.
int load_entries(const char *filename, DiaryEntry **entries);



// Search for an entry by title using binary search (sort the array with custom qsort).
// Returns the index of the entry, or -1 if not found.
int search_by_title_fast(const DiaryEntry *entries, int count, const char *title);

// Search for entries whose title contains the given substring (case-insensitive).
// Stores matching indices in results[] and returns the number of matches.
int search_by_title_partial(const DiaryEntry *entries, int count, const char *substring, int *results, int max_results);

// Search for all entries with an exact date match.
// Stores matching indices in results[] and returns the number of matches.
int search_by_date(const DiaryEntry *entries, int count, const char *date, int *results, int max_results);

// Search for all entries within a date range (inclusive).
// Stores matching indices in results[] and returns the number of matches.
int search_by_date_range(const DiaryEntry *entries, int count, const char *from, const char *to, int *results, int max_results);

// Sort entries by title. Uses qsort.
void sort_entries_by_title(DiaryEntry *entries, int count);

// Print a single entry in a formatted way.
void print_entry(const DiaryEntry *entry);

#endif