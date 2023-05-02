/* This file is part of stiv. */
/* Copyright (C) 2022 Lucas Mior */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.*/#include <unistd.h>

#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>

#include "stiv.h"

#define RD_EOF -1
#define RD_EIO -2

static int cursor_eread(const int);
static int cursor_ewrite(const int, const char *const, const size_t);
static int cursor_current_tty(void);
static int cursor_position(const int, int *const, int *const);

int cursor_eread(const int fd) {
    unsigned char buffer[4];
    ssize_t n;

    while (true) {
        n = read(fd, buffer, 1);

        if (n > (ssize_t) 0)
            return buffer[0];
        else if (n == (ssize_t) 0)
            return RD_EOF;
        else if (n != (ssize_t) -1)
            return RD_EIO;
        else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return RD_EIO;
    }
}

int cursor_ewrite(const int fd, const char *const data, const size_t bytes) {
    const char *head = data;
    const char *tail = data + bytes;
    ssize_t n;

    while (head < tail) {
        n = write(fd, head, (size_t)(tail - head));

        if (n > (ssize_t)0)
            head += n;
        else if (n != (ssize_t)(-1))
            return EIO;
        else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return errno;
    }
    return 0;
}

int cursor_current_tty(void) {
    const char *dev;
    int fd;

    dev = ttyname(STDIN_FILENO);
    if (!dev)
        dev = ttyname(STDOUT_FILENO);
    if (!dev)
        dev = ttyname(STDERR_FILENO);
    if (!dev) {
        errno = ENOTTY;
        return -1;
    }

    do {
        fd = open(dev, O_RDWR | O_NOCTTY);
    } while (fd == -1 && errno == EINTR);
    if (fd == -1)
        return -1;

    return fd;
}

int cursor_position(const int tty, int *const rowptr, int *const colptr) {
    struct termios saved, temporary;
    int retval, result, rows, cols, saved_errno;

    if (tty == -1)
        return ENOTTY;

    saved_errno = errno;

    /* Save current terminal settings. */
    do {
        result = tcgetattr(tty, &saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = saved_errno;
        return retval;
    }

    /* Get current terminal settings for basis, too. */
    do {
        result = tcgetattr(tty, &temporary);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = saved_errno;
        return retval;
    }

    /* Disable ICANON, ECHO, and CREAD. */
    temporary.c_lflag &= ~ICANON;
    temporary.c_lflag &= ~ECHO;
    temporary.c_cflag &= ~CREAD;

    /* This loop is only executed once. When broken out,
     * the terminal settings will be restored, and the function
     * will return retval to caller. It's better than goto.
    */
    do {
        /* Set modified settings. */
        do {
            result = tcsetattr(tty, TCSANOW, &temporary);
        } while (result == -1 && errno == EINTR);
        if (result == -1) {
            retval = errno;
            break;
        }

        /* Request cursor coordinates from the terminal. */
        retval = cursor_ewrite(tty, "\033[6n", 4);
        if (retval)
            break;

        /* Assume coordinate reponse parsing fails. */
        retval = EIO;

        /* Expect an ESC. */
        result = cursor_eread(tty);
        if (result != 27)
            break;

        /* Expect [ after the ESC. */
        result = cursor_eread(tty);
        if (result != '[')
            break;

        rows = 0;
        result = cursor_eread(tty);
        while (result >= '0' && result <= '9') {
            rows = 10 * rows + result - '0';
            result = cursor_eread(tty);
        }

        if (result != ';')
            break;

        cols = 0;
        result = cursor_eread(tty);
        while (result >= '0' && result <= '9') {
            cols = 10 * cols + result - '0';
            result = cursor_eread(tty);
        }

        if (result != 'R')
            break;

        if (rowptr)
            *rowptr = rows;

        if (colptr)
            *colptr = cols;

        retval = 0;
    } while (0);

    /* Restore saved terminal settings. */
    do {
        result = tcsetattr(tty, TCSANOW, &saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1 && !retval)
        retval = errno;

    return retval;
}

int cursor_getx(void) {
    int fd;
    int row = 0;
    int col = 0;

    fd = cursor_current_tty();
    if (fd == -1)
        return 1;

    if (cursor_position(fd, &row, &col))
        return 2;

    if (row < 1 || col < 1)
        return 3;

    return row;
}
