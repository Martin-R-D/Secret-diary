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
    entry.title[strcspn(entry.title, "\n")] = '\0'; // Strip newline

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

    // Step 1: Decrypt data file temporarily for streaming
    decrypt_file(filename, current_user);

    f = fopen(filename, "rb");
    if (!f) return;

    DiaryEntry entry;
    printf("\n==================== %s's DIARY HISTORY ====================", current_user);
    while (fread(&entry, sizeof(DiaryEntry), 1, f) == 1) {
        printf("\nDate:  %s\nTitle: %s", entry.date, entry.title);
        printf("\n---------------------------------------------------------");
        printf("\n%s\n", entry.body);
        printf("=========================================================");
    }
    fclose(f);

    // Step 2: Re-encrypt immediately to secure plain text signatures
    encrypt_file(filename, current_user);
}