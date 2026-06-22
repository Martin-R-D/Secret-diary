#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "users.h"

/* ------------------------------------------------------------------ */
/* Session state                                                        */
/* ------------------------------------------------------------------ */

char current_user[MAX_USERNAME_LEN] = {0};

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * simple_hash()
 *   Deterministic, portable password hash (djb2 variant iterated
 *   HASH_ITERATIONS times).  NOT cryptographic – replace with bcrypt
 *   or libsodium in a production build.
 *   Writes a hex string into `out` (must be >= 17 bytes).
 */
static void simple_hash(const char *password, const char *username,
                        char *out, size_t out_size)
{
    unsigned long hash = 5381;
    const char *p;
    int i;

    /* Mix in the username as salt */
    for (p = username; *p; p++)
        hash = ((hash << 5) + hash) ^ (unsigned char)*p;

    for (i = 0; i < HASH_ITERATIONS; i++) {
        for (p = password; *p; p++)
            hash = ((hash << 5) + hash) ^ (unsigned char)*p;
    }

    snprintf(out, out_size, "%016lx", hash);
}

/*
 * trim_newline()
 *   Strips trailing '\n' and '\r' from a string read with fgets().
 */
static void trim_newline(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

/*
 * username_valid()
 *   Usernames may only contain letters, digits, underscores, hyphens.
 *   Returns 1 if valid, 0 otherwise.
 */
static int username_valid(const char *name)
{
    if (!name || name[0] == '\0') return 0;
    for (; *name; name++) {
        if (!isalnum((unsigned char)*name) &&
            *name != '_' && *name != '-')
            return 0;
    }
    return 1;
}

/*
 * user_exists()
 *   Returns 1 if `username` is found in USERS_FILE, 0 otherwise.
 */
static int user_exists(const char *username)
{
    FILE *f = fopen(USERS_FILE, "r");
    char line_user[MAX_USERNAME_LEN];
    char line_hash[17];

    if (!f) return 0;   /* file doesn't exist yet → no users yet */

    while (fscanf(f, "%31s %16s", line_user, line_hash) == 2) {
        if (strcmp(line_user, username) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}
/*
 * read_password()
 * Reads a password from stdin without echoing.
 * Works cleanly on both Windows and POSIX systems.
 */
#ifdef _WIN32
#include <conio.h>
#endif

static void read_password(char *buf, size_t size)
{
#ifdef _WIN32
    size_t i = 0;
    int ch;
    // Read characters until Enter (13 or 10) is pressed
    while (i < size - 1) {
        ch = _getch();
        if (ch == 13 || ch == 10 || ch == EOF) {
            break;
        } else if (ch == 8 || ch == 127) { // Handle Backspace
            if (i > 0) {
                i--;
                printf("\b \b"); // Erase character from terminal visual
            }
        } else {
            buf[i++] = (char)ch;
            printf("*"); // Print a mask instead of plain text
        }
    }
    buf[i] = '\0';
    printf("\n");
#elif defined(_POSIX_C_SOURCE)
    /* Disable echo for Linux/macOS systems */
    #include <termios.h>
    struct termios oldt, newt;
    tcgetattr(fileno(stdin), &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(fileno(stdin), TCSANOW, &newt);
    fgets(buf, (int)size, stdin);
    tcsetattr(fileno(stdin), TCSANOW, &oldt);
    printf("\n");
    trim_newline(buf);
#else
    // Fallback if anything else
    fgets(buf, (int)size, stdin);
    trim_newline(buf);
#endif
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

int register_user(void)
{
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char confirm[MAX_PASSWORD_LEN];
    char hash[17];
    FILE *f;

    /* --- get username --- */
    printf("Choose a username (letters, digits, _ or -): ");
    if (!fgets(username, sizeof(username), stdin)) return -2;
    trim_newline(username);

    if (!username_valid(username)) {
        printf("Invalid username. Use only letters, digits, '_', or '-'.\n");
        return -1;
    }

    if (user_exists(username)) {
        printf("Username '%s' is already taken.\n", username);
        return -1;
    }

    /* --- get password --- */
    printf("Choose a password: ");
    read_password(password, sizeof(password));

    if (strlen(password) < 4) {
        printf("Password must be at least 4 characters.\n");
        return -1;
    }

    printf("Confirm password: ");
    read_password(confirm, sizeof(confirm));

    if (strcmp(password, confirm) != 0) {
        printf("Passwords do not match.\n");
        /* Clear sensitive data */
        memset(password, 0, sizeof(password));
        memset(confirm,  0, sizeof(confirm));
        return -1;
    }

    /* --- store hashed credentials --- */
    simple_hash(password, username, hash, sizeof(hash));
    memset(password, 0, sizeof(password));
    memset(confirm,  0, sizeof(confirm));

    f = fopen(USERS_FILE, "a");
    if (!f) {
        perror("Cannot open users file");
        return -2;
    }
    fprintf(f, "%s %s\n", username, hash);
    fclose(f);

    printf("Account created for '%s'.\n", username);
    return 0;
}

int login_user(void)
{
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char input_hash[17];
    char line_user[MAX_USERNAME_LEN];
    char line_hash[17];
    FILE *f;
    int  found = 0;

    if (is_logged_in()) {
        printf("Already logged in as '%s'. Log out first.\n", current_user);
        return -1;
    }

    printf("Username: ");
    if (!fgets(username, sizeof(username), stdin)) return -2;
    trim_newline(username);

    printf("Password: ");
    read_password(password, sizeof(password));

    simple_hash(password, username, input_hash, sizeof(input_hash));
    memset(password, 0, sizeof(password));

    f = fopen(USERS_FILE, "r");
    if (!f) {
        printf("No accounts registered yet.\n");
        return -2;
    }

    while (fscanf(f, "%31s %16s", line_user, line_hash) == 2) {
        if (strcmp(line_user, username) == 0 &&
            strcmp(line_hash, input_hash) == 0) {
            found = 1;
            break;
        }
    }
    fclose(f);

    if (!found) {
        printf("Invalid username or password.\n");
        return -1;
    }

    strncpy(current_user, username, MAX_USERNAME_LEN - 1);
    current_user[MAX_USERNAME_LEN - 1] = '\0';
    printf("Welcome back, %s!\n", current_user);
    return 0;
}

void logout_user(void)
{
    if (!is_logged_in()) {
        printf("No user is currently logged in.\n");
        return;
    }
    printf("Goodbye, %s!\n", current_user);
    memset(current_user, 0, sizeof(current_user));
}

int is_logged_in(void)
{
    return current_user[0] != '\0';
}

char *get_diary_filename(char *buf, size_t buf_size)
{
    if (!is_logged_in()) return NULL;
    snprintf(buf, buf_size, "diary_%s.dat", current_user);
    return buf;
}