/*
 * Project 5: Library Book Management System
 * Topics : Arrays, Structures, Functions, Pointers (+ dynamic memory)
 *
 * Features
 *  - Book record: ID, Title, Author, Availability
 *  - Dynamic array of structures (malloc / realloc / free)
 *  - Add, display, search (by ID / title / author), update, delete
 *  - Issue and return books
 *  - Availability report
 *  - Every function receives pointers (no copying of whole records)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TITLE_LEN  100
#define AUTHOR_LEN  60
#define INIT_CAP     4

/* ---------- Data structures ---------- */

typedef struct {
    int  id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    int  available;          /* 1 = available, 0 = issued */
} Book;

typedef struct {
    Book *books;             /* dynamically allocated array of Book */
    int   count;             /* books currently stored */
    int   capacity;          /* allocated slots */
} Library;

/* ---------- Input helpers ---------- */

void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = '\0';
}

int readInt(const char *prompt) {
    char line[32];
    long val;
    char *end;
    for (;;) {
        readLine(prompt, line, sizeof line);
        val = strtol(line, &end, 10);
        if (end != line && *end == '\0') return (int)val;
        printf("  Invalid number, try again.\n");
    }
}

/* Case-insensitive substring test */
int containsIgnoreCase(const char *text, const char *pattern) {
    size_t n = strlen(text), m = strlen(pattern);
    if (m == 0) return 1;
    for (size_t i = 0; i + m <= n; i++) {
        size_t j = 0;
        while (j < m && tolower((unsigned char)text[i + j]) ==
                        tolower((unsigned char)pattern[j])) j++;
        if (j == m) return 1;
    }
    return 0;
}

/* ---------- Library management ---------- */

int initLibrary(Library *lib) {
    lib->books = malloc(INIT_CAP * sizeof(Book));
    lib->count = 0;
    lib->capacity = lib->books ? INIT_CAP : 0;
    return lib->books != NULL;
}

void freeLibrary(Library *lib) {
    free(lib->books);
    lib->books = NULL;
    lib->count = lib->capacity = 0;
}

/* Returns pointer to the book with this ID, or NULL */
Book *findById(Library *lib, int id) {
    for (Book *p = lib->books; p < lib->books + lib->count; p++)
        if (p->id == id) return p;
    return NULL;
}

void printBook(const Book *b) {
    printf("%-6d %-30.30s %-22.22s %s\n",
           b->id, b->title, b->author,
           b->available ? "Available" : "Issued");
}

void printHeader(void) {
    printf("\n%-6s %-30s %-22s %s\n", "ID", "Title", "Author", "Status");
    printf("---------------------------------------------------------------------\n");
}

void addBook(Library *lib) {
    int id = readInt("Book ID: ");
    if (id <= 0) { printf("  ID must be positive.\n"); return; }
    if (findById(lib, id)) { printf("  A book with ID %d already exists.\n", id); return; }

    if (lib->count == lib->capacity) {            /* grow dynamically */
        int newCap = lib->capacity ? lib->capacity * 2 : INIT_CAP;
        Book *tmp = realloc(lib->books, newCap * sizeof(Book));
        if (!tmp) { printf("  Out of memory.\n"); return; }
        lib->books = tmp;
        lib->capacity = newCap;
    }

    Book *b = &lib->books[lib->count];
    b->id = id;
    readLine("Title : ", b->title, TITLE_LEN);
    readLine("Author: ", b->author, AUTHOR_LEN);
    b->available = 1;
    lib->count++;
    printf("  Book added.\n");
}

void displayAll(Library *lib) {
    if (lib->count == 0) { printf("  Catalogue is empty.\n"); return; }
    printHeader();
    for (Book *p = lib->books; p < lib->books + lib->count; p++) printBook(p);
}

void searchBooks(Library *lib) {
    printf("Search by: 1) ID  2) Title  3) Author\n");
    int choice = readInt("Choice: ");
    int found = 0;

    if (choice == 1) {
        Book *b = findById(lib, readInt("Enter ID: "));
        if (b) { printHeader(); printBook(b); found = 1; }
    } else if (choice == 2 || choice == 3) {
        char key[TITLE_LEN];
        readLine("Keyword: ", key, sizeof key);
        for (Book *p = lib->books; p < lib->books + lib->count; p++) {
            const char *field = (choice == 2) ? p->title : p->author;
            if (containsIgnoreCase(field, key)) {
                if (!found) printHeader();
                printBook(p);
                found = 1;
            }
        }
    } else {
        printf("  Invalid choice.\n");
        return;
    }
    if (!found) printf("  No matching book found.\n");
}

void issueBook(Library *lib) {
    Book *b = findById(lib, readInt("Book ID to issue: "));
    if (!b)               printf("  Book not found.\n");
    else if (!b->available) printf("  '%s' is already issued.\n", b->title);
    else { b->available = 0; printf("  Issued '%s'.\n", b->title); }
}

void returnBook(Library *lib) {
    Book *b = findById(lib, readInt("Book ID to return: "));
    if (!b)               printf("  Book not found.\n");
    else if (b->available)  printf("  '%s' was not issued.\n", b->title);
    else { b->available = 1; printf("  Returned '%s'.\n", b->title); }
}

void updateBook(Library *lib) {
    Book *b = findById(lib, readInt("Book ID to update: "));
    if (!b) { printf("  Book not found.\n"); return; }
    char buf[TITLE_LEN];
    printf("  (Press Enter to keep the current value)\n");
    printf("  Current title : %s\n", b->title);
    readLine("  New title     : ", buf, TITLE_LEN);
    if (buf[0]) strcpy(b->title, buf);
    printf("  Current author: %s\n", b->author);
    readLine("  New author    : ", buf, AUTHOR_LEN);
    if (buf[0]) strcpy(b->author, buf);
    printf("  Book updated.\n");
}

void deleteBook(Library *lib) {
    Book *b = findById(lib, readInt("Book ID to delete: "));
    if (!b) { printf("  Book not found.\n"); return; }
    Book *last = lib->books + lib->count - 1;
    for (Book *p = b; p < last; p++) *p = *(p + 1);   /* shift left */
    lib->count--;
    printf("  Book deleted.\n");
}

void availabilityReport(Library *lib) {
    int avail = 0;
    for (Book *p = lib->books; p < lib->books + lib->count; p++) avail += p->available;
    printf("\n===== Availability Report =====\n");
    printf("Total books : %d\n", lib->count);
    printf("Available   : %d\n", avail);
    printf("Issued      : %d\n", lib->count - avail);

    if (lib->count - avail > 0) {
        printf("\nCurrently issued:");
        printHeader();
        for (Book *p = lib->books; p < lib->books + lib->count; p++)
            if (!p->available) printBook(p);
    }
}

/* ---------- Main menu ---------- */

void showMenu(void) {
    printf("\n===== Library Book Management =====\n"
           " 1. Add book\n"
           " 2. Display all books\n"
           " 3. Search book\n"
           " 4. Issue book\n"
           " 5. Return book\n"
           " 6. Update book\n"
           " 7. Delete book\n"
           " 8. Availability report\n"
           " 0. Exit\n");
}

int main(void) {
    Library lib;
    if (!initLibrary(&lib)) { fprintf(stderr, "Memory allocation failed.\n"); return 1; }

    int choice;
    do {
        showMenu();
        choice = readInt("Enter choice: ");
        switch (choice) {
            case 1: addBook(&lib);            break;
            case 2: displayAll(&lib);         break;
            case 3: searchBooks(&lib);        break;
            case 4: issueBook(&lib);          break;
            case 5: returnBook(&lib);         break;
            case 6: updateBook(&lib);         break;
            case 7: deleteBook(&lib);         break;
            case 8: availabilityReport(&lib); break;
            case 0: printf("Goodbye!\n");     break;
            default: printf("  Invalid choice.\n");
        }
    } while (choice != 0);

    freeLibrary(&lib);
    return 0;
}
