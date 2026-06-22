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

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(sorted[i].title, sorted[j].title) > 0) {
                DiaryEntry tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
                int t = index_map[i];
                index_map[i] = index_map[j];
                index_map[j] = t;
            }
        }
    }

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
