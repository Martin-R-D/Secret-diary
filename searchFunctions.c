#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "searchFunctions.h"

int load_entries(const char *filename, DiaryEntry **entries) {
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return -1;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0 || file_size % sizeof(DiaryEntry) != 0) {
        fclose(fp);
        return -1;
    }

    int count = (int)(file_size / sizeof(DiaryEntry));
    *entries = malloc(count * sizeof(DiaryEntry));
    if (!*entries) {
        fclose(fp);
        return -1;
    }

    size_t read = fread(*entries, sizeof(DiaryEntry), count, fp);
    fclose(fp);

    if ((int)read != count) {
        free(*entries);
        *entries = NULL;
        return -1;
    }

    return count;
}

static int compare_by_title(const void *a, const void *b) {
    return strcmp(((const DiaryEntry *)a)->title, ((const DiaryEntry *)b)->title);
}

int search_by_title_fast(const DiaryEntry *entries, int count, const char *title) {
    DiaryEntry *sorted = malloc(count * sizeof(DiaryEntry));
    if (!sorted)
        return -1;

    memcpy(sorted, entries, count * sizeof(DiaryEntry));

    int *index_map = malloc(count * sizeof(int));
    if (!index_map) {
        free(sorted);
        return -1;
    }
    for (int i = 0; i < count; i++)
        index_map[i] = i;

    DiaryEntry *temp_entries = malloc(count * sizeof(DiaryEntry));
    int *temp_indices = malloc(count * sizeof(int));
    if (!temp_entries || !temp_indices) {
        free(temp_entries);
        free(temp_indices);
        free(sorted);
        free(index_map);
        return -1;
    }

    for (int width = 1; width < count; width *= 2) {
        for (int i = 0; i < count; i += 2 * width) {
            int left = i;
            int mid = i + width;
            if (mid > count) mid = count;
            int right = i + 2 * width;
            if (right > count) right = count;

            int l = left, r = mid, k = left;
            while (l < mid && r < right) {
                if (strcmp(sorted[l].title, sorted[r].title) <= 0) {
                    temp_entries[k] = sorted[l];
                    temp_indices[k] = index_map[l];
                    l++;
                } else {
                    temp_entries[k] = sorted[r];
                    temp_indices[k] = index_map[r];
                    r++;
                }
                k++;
            }
            while (l < mid) {
                temp_entries[k] = sorted[l];
                temp_indices[k] = index_map[l];
                l++;
                k++;
            }
            while (r < right) {
                temp_entries[k] = sorted[r];
                temp_indices[k] = index_map[r];
                r++;
                k++;
            }
        }
        memcpy(sorted, temp_entries, count * sizeof(DiaryEntry));
        memcpy(index_map, temp_indices, count * sizeof(int));
    }

    free(temp_entries);
    free(temp_indices);

    int lo = 0, hi = count - 1, result = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(sorted[mid].title, title);
        if (cmp == 0) {
            result = index_map[mid];
            break;
        } else if (cmp < 0) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    free(sorted);
    free(index_map);
    return result;
}

int search_by_title_partial(const DiaryEntry *entries, int count, const char *substring, int *results, int max_results) {
    int matches = 0;
    int sub_len = (int)strlen(substring);

    for (int i = 0; i < count && matches < max_results; i++) {
        int title_len = (int)strlen(entries[i].title);
        for (int j = 0; j <= title_len - sub_len; j++) {
            int found = 1;
            for (int k = 0; k < sub_len; k++) {
                if (tolower((unsigned char)entries[i].title[j + k]) != tolower((unsigned char)substring[k])) {
                    found = 0;
                    break;
                }
            }
            if (found) {
                results[matches++] = i;
                break;
            }
        }
    }

    return matches;
}

int search_by_date(const DiaryEntry *entries, int count, const char *date, int *results, int max_results) {
    DiaryEntry *sorted = malloc(count * sizeof(DiaryEntry));
    if (!sorted)
        return 0;

    int *index_map = malloc(count * sizeof(int));
    if (!index_map) {
        free(sorted);
        return 0;
    }

    memcpy(sorted, entries, count * sizeof(DiaryEntry));
    for (int i = 0; i < count; i++)
        index_map[i] = i;

    DiaryEntry *temp_entries = malloc(count * sizeof(DiaryEntry));
    int *temp_indices = malloc(count * sizeof(int));
    if (!temp_entries || !temp_indices) {
        free(temp_entries);
        free(temp_indices);
        free(sorted);
        free(index_map);
        return 0;
    }

    for (int width = 1; width < count; width *= 2) {
        for (int i = 0; i < count; i += 2 * width) {
            int left = i;
            int mid = i + width;
            if (mid > count) mid = count;
            int right = i + 2 * width;
            if (right > count) right = count;

            int l = left, r = mid, k = left;
            while (l < mid && r < right) {
                if (strcmp(sorted[l].date, sorted[r].date) <= 0) {
                    temp_entries[k] = sorted[l];
                    temp_indices[k] = index_map[l];
                    l++;
                } else {
                    temp_entries[k] = sorted[r];
                    temp_indices[k] = index_map[r];
                    r++;
                }
                k++;
            }
            while (l < mid) {
                temp_entries[k] = sorted[l];
                temp_indices[k] = index_map[l];
                l++;
                k++;
            }
            while (r < right) {
                temp_entries[k] = sorted[r];
                temp_indices[k] = index_map[r];
                r++;
                k++;
            }
        }
        memcpy(sorted, temp_entries, count * sizeof(DiaryEntry));
        memcpy(index_map, temp_indices, count * sizeof(int));
    }

    free(temp_entries);
    free(temp_indices);

    int lo = 0, hi = count - 1, found = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(sorted[mid].date, date);
        if (cmp == 0) {
            found = mid;
            break;
        } else if (cmp < 0) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    int matches = 0;
    if (found != -1) {
        int start = found;
        while (start > 0 && strcmp(sorted[start - 1].date, date) == 0)
            start--;
        int end = found;
        while (end < count - 1 && strcmp(sorted[end + 1].date, date) == 0)
            end++;
        for (int i = start; i <= end && matches < max_results; i++)
            results[matches++] = index_map[i];
    }

    free(sorted);
    free(index_map);
    return matches;
}

int search_by_date_range(const DiaryEntry *entries, int count, const char *from, const char *to, int *results, int max_results) {
    int matches = 0;
    for (int i = 0; i < count && matches < max_results; i++) {
        if (strcmp(entries[i].date, from) >= 0 && strcmp(entries[i].date, to) <= 0)
            results[matches++] = i;
    }
    return matches;
}

void sort_entries_by_title(DiaryEntry *entries, int count) {
    qsort(entries, count, sizeof(DiaryEntry), compare_by_title);
}

void print_entry(const DiaryEntry *entry) {
    printf("Title: %s\n", entry->title);
    printf("Date:  %s\n", entry->date);
    printf("Body:\n%s\n", entry->body);
    printf("---\n");
}
