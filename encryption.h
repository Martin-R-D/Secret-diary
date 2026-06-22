#ifndef ENCRYPTION_H
#define ENCRYPTION_H

void encrypt_file(const char *filename, const char *password);
void decrypt_file(const char *filename, const char *password);

#endif