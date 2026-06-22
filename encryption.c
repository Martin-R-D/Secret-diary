#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encryption.h"

/*
 * Derives a single-byte key stream from the password using a simple
 * rolling XOR hash.  For every byte position i we mix in the password
 * character at (i % password_length) and a position-dependent constant
 * so that the key never stays flat even with short passwords.
 */
static unsigned char key_byte(const char *password, size_t pass_len, size_t pos) {
    unsigned char k = (unsigned char)password[pos % pass_len];
    k ^= (unsigned char)(pos * 31 + 7);   /* position mixing */
    k ^= (unsigned char)(pass_len * 17);  /* length mixing   */
    return k ? k : 0xAB;                  /* never emit 0 – XOR with 0 is identity */
}

/* XOR-encrypt src → dst using the password-derived key stream. */
static void xor_crypt(const unsigned char *src, unsigned char *dst,
                      size_t len, const char *password) {
    size_t pass_len = strlen(password);
    for (size_t i = 0; i < len; i++) {
        dst[i] = src[i] ^ key_byte(password, pass_len, i);
    }
}

/* ------------------------------------------------------------------
 * encrypt_file  –  reads filename, encrypts in-place with password
 * ------------------------------------------------------------------ */
void encrypt_file(const char *filename, const char *password) {
    FILE *f = fopen(filename, "rb");
    if (!f) { perror("encrypt_file: open"); return; }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    unsigned char *buf = malloc(size);
    if (!buf) { fclose(f); return; }

    fread(buf, 1, size, f);
    fclose(f);

    xor_crypt(buf, buf, size, password);   /* encrypt in-place */

    f = fopen(filename, "wb");
    if (!f) { perror("encrypt_file: write"); free(buf); return; }
    fwrite(buf, 1, size, f);
    fclose(f);
    free(buf);
}

/* ------------------------------------------------------------------
 * decrypt_file  –  XOR is symmetric, so decryption == encryption
 * ------------------------------------------------------------------ */
void decrypt_file(const char *filename, const char *password) {
    encrypt_file(filename, password);   /* same operation */
}