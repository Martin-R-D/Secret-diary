# Secret Diary

A terminal-based encrypted diary application written in C.

## About

Secret Diary lets multiple users keep personal journals protected by password authentication and file encryption. Each user can create diary entries with a title (up to 30 characters), date, and body text, and later search through them by title or date.

## Features

- **User Registration & Login** — each user has their own account with a hashed password
- **Create & Read Entries** — write new diary entries and browse existing ones
- **Encryption** — diary files are encrypted per-user using the username as the encryption key
- **Search** — exact title search (merge sort + binary search), partial case-insensitive title search, exact date match, and date range filtering
- **Multi-User Support** — multiple people can use the same application, each with a separate encrypted diary
