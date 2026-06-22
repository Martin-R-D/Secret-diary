#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "users.h"
#include "diary.h"
#include "searchFunctions.h"
#include "encryption.h"

void handle_search(void) {
    if (!is_logged_in()) {
        printf("Please log in first.\n");
        return;
    }

    char filename[64];
    get_diary_filename(filename, sizeof(filename));

    // Temporarily decrypt file data for search routines
    FILE *check = fopen(filename, "rb");
    if (!check) {
        printf("No records found to search through.\n");
        return;
    }
    fclose(check);

    decrypt_file(filename, current_user);

    DiaryEntry *entries = NULL;
    int count = load_entries(filename, &entries);

    if (count <= 0) {
        printf("No valid entries loaded.\n");
        encrypt_file(filename, current_user);
        return;
    }

    int choice;
    printf("\n--- SEARCH DIARY ---\n");
    printf("1. Quick exact search by Title\n");
    printf("2. Partial match search by Title\n");
    printf("3. Filter by Exact Date\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    getchar();

    char term[100];
    printf("Enter search term: ");
    fgets(term, sizeof(term), stdin);
    term[strcspn(term, "\n")] = '\0';

    if (choice == 1) {
        int idx = search_by_title_fast(entries, count, term);
        if (idx != -1) print_entry(&entries[idx]);
        else printf("No exact matches found.\n");
    } else if (choice == 2) {
        int results[100];
        int matches = search_by_title_partial(entries, count, term, results, 100);
        for(int i = 0; i < matches; i++) print_entry(&entries[results[i]]);
        if(matches == 0) printf("No results found.\n");
    } else if (choice == 3) {
        int results[100];
        int matches = search_by_date(entries, count, term, results, 100);
        for(int i = 0; i < matches; i++) print_entry(&entries[results[i]]);
        if(matches == 0) printf("No results found.\n");
    }

    free(entries);
    // Secure back original file configurations
    encrypt_file(filename, current_user);
}

int main(void) {
    int choice;
    while (1) {
        printf("\n=== SECRET DIARY SYSTEM ===\n");
        if (is_logged_in()) {
            printf("Active User Session: [%s]\n", current_user);
            printf("1. Write New Diary Entry\n");
            printf("2. Read Diary History\n");
            printf("3. Search Through History\n");
            printf("4. Logout\n");
        } else {
            printf("Status: Not Logged In\n");
            printf("1. Login\n");
            printf("2. Register New Account\n");
        }
        printf("0. Exit Application\n");
        printf("Select option: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid selection typing type.\n");
            break;
        }
        getchar(); // clear out system buffers

        if (choice == 0) {
            if (is_logged_in()) logout_user();
            printf("Exiting Diary Application. Goodbye.\n");
            break;
        }

        if (is_logged_in()) {
            switch (choice) {
                case 1: add_diary_entry(); break;
                case 2: view_diary_history(); break;
                case 3: handle_search(); break;
                case 4: logout_user(); break;
                default: printf("Option error. Select valid numbers.\n");
            }
        } else {
            switch (choice) {
                case 1: login_user(); break;
                case 2: register_user(); break;
                default: printf("Option error. Select valid numbers.\n");
            }
        }
    }
    return 0;
}