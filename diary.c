#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "searchFunctions.h"  // Contains DiaryEntry definition
#include "users.h"            // Contains user session data
#include "encryption.h"       // Contains file locking/unlocking 
#include "diary.h"

void add_diary_entry(void) {
    // Check if a session exists
    if (!is_logged_in()) {
        printf("Error: You must be logged in to write to the diary!\n");
        return;
    }

    char filename[64];
    get_diary_filename(filename, sizeof(filename));

    DiaryEntry entry;
    memset(&entry, 0, sizeof(DiaryEntry));

    printf("\n--- CREATE NEW DIARY ENTRY ---\n");
    printf("Enter date (YYYY-MM-DD): ");
    scanf("%10s", entry.date);
    getchar(); // Clear trailing newline from buffer

    printf("Enter title: ");
    fgets(entry.title, sizeof(entry.title), stdin);
    entry.title[strcspn(entry.title, "\n")] = '\0';

    if (strlen(entry.title) > 30) {
        printf("Title must be 30 characters or less.\n");
        return;
    }

    printf("Enter content body:\n");
    fgets(entry.body, sizeof(entry.body), stdin);
    entry.body[strcspn(entry.body, "\n")] = '\0'; // Strip newline

    // Step 1: Temporarily decrypt the existing file if it exists
    // We pass the username as the file encryption key
    FILE *check = fopen(filename, "rb");
    if (check) {
        fclose(check);
        decrypt_file(filename, current_user);
    }

    // Step 2: Append the raw unencrypted structure data
    FILE *f = fopen(filename, "ab");
    if (!f) {
        printf("Error: Failed to open diary storage file.\n");
        return;
    }
    fwrite(&entry, sizeof(DiaryEntry), 1, f);
    fclose(f);

    // Step 3: Encrypt the entire file securely back to disk
    encrypt_file(filename, current_user);

    printf("\n[+] Success: Story encrypted and saved to %s!\n", filename);
}

void view_diary_history(void) {
    if (!is_logged_in()) {
        printf("Error: You must be logged in to view history!\n");
        return;
    }

    char filename[64];
    get_diary_filename(filename, sizeof(filename));

    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("\nYour diary is currently empty. Start writing some memories!\n");
        return;
    }
    fclose(f);

    decrypt_file(filename, current_user);

    DiaryEntry *entries = NULL;
    int count = load_entries(filename, &entries);

    if (count <= 0) {
        printf("\nYour diary is currently empty. Start writing some memories!\n");
        encrypt_file(filename, current_user);
        return;
    }

    printf("\n==================== %s's DIARY HISTORY ====================\n", current_user);
    for (int i = 0; i < count; i++) {
        printf("  %d. [%s] %s\n", i + 1, entries[i].date, entries[i].title);
    }
    printf("=========================================================\n");

    printf("Enter entry number to read (0 to go back): ");
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 0 || choice > count) {
        printf("Invalid selection.\n");
        while (getchar() != '\n');
        free(entries);
        encrypt_file(filename, current_user);
        return;
    }
    getchar();

    if (choice > 0) {
        printf("\n");
        print_entry(&entries[choice - 1]);
    }

    free(entries);
    encrypt_file(filename, current_user);
}