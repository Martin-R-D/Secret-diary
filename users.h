#ifndef USERS_H
#define USERS_H

#define MAX_USERNAME_LEN  32
#define MAX_PASSWORD_LEN  64
#define MAX_USERS         100
#define USERS_FILE        "users.dat"
#define HASH_ITERATIONS   1000

/* Holds the currently logged-in user for the session */
extern char current_user[MAX_USERNAME_LEN];

/*
 * register_user()
 *   Prompts for a username and password.
 *   Returns  0 on success,
 *           -1 if the username already exists,
 *           -2 on I/O error.
 */
int register_user(void);

/*
 * login_user()
 *   Prompts for credentials and validates them against users.dat.
 *   On success, sets current_user and returns 0.
 *   Returns -1 on wrong credentials, -2 on I/O error.
 */
int login_user(void);

/*
 * logout_user()
 *   Clears current_user, ending the session.
 */
void logout_user(void);

/*
 * is_logged_in()
 *   Returns 1 if a user is currently logged in, 0 otherwise.
 */
int is_logged_in(void);

/*
 * get_diary_filename()
 *   Writes the path for the current user's diary file into `buf`.
 *   buf must be at least MAX_USERNAME_LEN + 16 bytes.
 *   Returns buf, or NULL if no user is logged in.
 */
char *get_diary_filename(char *buf, size_t buf_size);

#endif /* USERS_H */