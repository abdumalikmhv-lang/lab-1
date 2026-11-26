#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
    KIND_DIR,
    KIND_TEXT,
    KIND_EMPTY,
    KIND_ZERO,
    KIND_RANDOM,
    KIND_HARDLINK,
    KIND_SYMLINK
} EntryKind;

typedef struct {
    EntryKind kind;
    const char *path;     /* Путь создаваемого элемента */
    const char *target;   /* Текст для KIND_TEXT или путь назначения для ссылок */
    off_t size;           /* Размер для KIND_ZERO и KIND_RANDOM */
} Entry;

/* Пример структуры из задания (подставьте свой вариант на сдаче) */
static const Entry ENTRIES[] = {
    { KIND_DIR,      "a",          NULL,           0   },
    { KIND_DIR,      "a/b",        NULL,           0   },
    { KIND_DIR,      "a/c",        NULL,           0   },
    { KIND_DIR,      "a/e",        NULL,           0   },
    { KIND_SYMLINK,  "a/d",        "c",            0   },
    { KIND_TEXT,     "a/b/cat.txt","catts",        0   },
    { KIND_EMPTY,    "a/c/dog.txt",NULL,           0   },
    { KIND_HARDLINK, "a/e/f.txt",  "../b/cat.txt", 0   },
    { KIND_RANDOM,   "a/e/g.bin",  NULL,           500 },
    { KIND_ZERO,     "a/e/h.bin",  NULL,           35  },
};

static int write_all(int fd, const void *buf, size_t count) {
    const char *p = (const char *)buf;
    size_t left = count;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            return -1;
        }
        p += (size_t)n;
        left -= (size_t)n;
    }
    return 0;
}

static int make_text(const char *path, const char *text) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    if (text != NULL && write_all(fd, text, strlen(text)) < 0) {
        perror(path);
        close(fd);
        return -1;
    }
    if (close(fd) < 0) {
        perror(path);
        return -1;
    }
    return 0;
}

static int make_empty(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    if (close(fd) < 0) {
        perror(path);
        return -1;
    }
    return 0;
}

static int make_zero(const char *path, off_t size) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    if (ftruncate(fd, size) < 0) {
        perror(path);
        close(fd);
        return -1;
    }
    if (close(fd) < 0) {
        perror(path);
        return -1;
    }
    return 0;
}

static int make_random(const char *path, off_t size) {
    if (size < 0) {
        return -1;
    }
    int src = open("/dev/urandom", O_RDONLY, S_IRWXU);
    if (src < 0) {
        perror("/dev/urandom");
        return -1;
    }
    int dst = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (dst < 0) {
        perror(path);
        close(src);
        return -1;
    }

    char buf[4096];
    size_t left = (size_t)size;
    while (left > 0) {
        size_t chunk = left < sizeof(buf) ? left : sizeof(buf);
        ssize_t r = read(src, buf, chunk);
        if (r <= 0) {
            perror("read /dev/urandom");
            close(src);
            close(dst);
            return -1;
        }
        if (write_all(dst, buf, (size_t)r) < 0) {
            perror(path);
            close(src);
            close(dst);
            return -1;
        }
        left -= (size_t)r;
    }

    close(src);
    if (close(dst) < 0) {
        perror(path);
        return -1;
    }
    return 0;
}

int main(void) {
    size_t count = sizeof(ENTRIES) / sizeof(ENTRIES[0]);
    for (size_t i = 0; i < count; i++) {
        const Entry *e = &ENTRIES[i];
        int rc = 0;

        switch (e->kind) {
            case KIND_DIR:
                rc = mkdir(e->path, S_IRWXU);
                break;
            case KIND_TEXT:
                rc = make_text(e->path, e->target);
                break;
            case KIND_EMPTY:
                rc = make_empty(e->path);
                break;
            case KIND_ZERO:
                rc = make_zero(e->path, e->size);
                break;
            case KIND_RANDOM:
                rc = make_random(e->path, e->size);
                break;
            case KIND_HARDLINK:
                rc = link(e->target, e->path);
                break;
            case KIND_SYMLINK:
                rc = symlink(e->target, e->path);
                break;
        }

        if (rc < 0) {
            perror(e->path);
            return 1;
        }
    }
    return 0;
}
