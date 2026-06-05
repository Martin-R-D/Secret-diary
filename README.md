# Secret Diary

A terminal-based encrypted diary application written in C.

## About

Secret Diary lets multiple users keep personal journals protected by password authentication and file encryption. Each user can create diary entries with a title, date, and body text, and later search through them quickly by title or date.

## Features

- **User Registration & Login** — each user has their own account with a hashed password
- **Create & Read Entries** — write new diary stories and browse existing ones
- **Encryption** — diary files are encrypted with a user-provided password so entries stay private on disk
- **Fast Search** — find entries by title (using binary search) or filter by date and date range
- **Multi-User Support** — multiple people can use the same application, each with a separate encrypted diary
