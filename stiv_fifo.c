#if !defined(STIV_FIFO_C)
#define STIV_FIFO_C

#include "cbase.h"
#include "stiv.h"

#define STIV_FIFO_RETRIES 20
#define STIV_FIFO_RETRY_MS 10

typedef struct StivFifoState {
    int64 seconds;
    int64 nanoseconds;
    int32 paused;
    int32 unused;
} StivFifoState;

static int32
stiv_fifo_open(char *fifo) {
    int32 fd;
    int32 err = ENXIO;

    for (int32 i = 0; i < STIV_FIFO_RETRIES; i += 1) {
        fd = open(fifo, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd >= 0) {
            return fd;
        }
        err = errno;
        if ((err != ENXIO) && (err != EINTR)) {
            return -err;
        }
        sleep_ms(STIV_FIFO_RETRY_MS);
    }
    return -err;
}

static int32
stiv_fifo_write(char *fifo, char *message, int32 message_len) {
    int32 fd;
    int32 err = 0;
    ssize_t written;
    struct sigaction ignore = {0};
    struct sigaction old_action;
    struct pollfd ready = {0};
    long pipe_buf;

    if ((fd = stiv_fifo_open(fifo)) < 0) {
        return fd;
    }
    pipe_buf = fpathconf(fd, _PC_PIPE_BUF);
    if (pipe_buf < 0) {
        err = errno ? errno : EIO;
        goto close_fifo;
    }
    if (message_len > pipe_buf) {
        err = EMSGSIZE;
        goto close_fifo;
    }

    ignore.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &ignore, &old_action) < 0) {
        err = errno;
        goto close_fifo;
    }

    ready.fd = fd;
    ready.events = POLLOUT;
    for (int32 i = 0; i < STIV_FIFO_RETRIES; i += 1) {
        written = write(fd, message, (size_t)message_len);
        if (written == message_len) {
            break;
        }
        if (written >= 0) {
            err = EIO;
            break;
        }
        err = errno;
        if (err == EINTR) {
            continue;
        }
        if (err != EAGAIN) {
            break;
        }
        if (poll(&ready, 1, STIV_FIFO_RETRY_MS) < 0) {
            err = errno;
            if (err != EINTR) {
                break;
            }
        }
    }
    if ((written != message_len) && (err == EAGAIN)) {
        err = ETIMEDOUT;
    }
    if (written == message_len) {
        err = 0;
    }
    if ((sigaction(SIGPIPE, &old_action, NULL) < 0) && (err == 0)) {
        err = errno;
    }

close_fifo:
    if ((close(fd) < 0) && (err == 0)) {
        err = errno;
    }
    return -err;
}

int32
stiv_fifo_send(enum StivFifoAction action,
               struct timespec started, char *message, int32 message_len) {
    StivFifoState state = {0};
    char *fifo = getenv("UEBERZUG_FIFO");
    char lock_path[PATH_MAX];
    int32 lock_fd;
    int32 err = 0;
    int32 path_len;
    ssize_t read_len;

    if ((fifo == NULL) || (fifo[0] == '\0')) {
        return -ENOENT;
    }
    path_len = snprintf(lock_path, sizeof(lock_path), "%s.lock", fifo);
    if ((path_len < 0) || (path_len >= (int32)sizeof(lock_path))) {
        return -ENAMETOOLONG;
    }
    lock_fd = open(lock_path, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
    if (lock_fd < 0) {
        return -errno;
    }
    while (flock(lock_fd, LOCK_EX) < 0) {
        if (errno != EINTR) {
            err = errno;
            goto close_lock;
        }
    }

    read_len = pread(lock_fd, &state, sizeof(state), 0);
    if (read_len < 0) {
        err = errno;
        goto close_lock;
    }
    if ((read_len != 0) && (read_len != sizeof(state))) {
        err = EIO;
        goto close_lock;
    }

    /* A late draw must not restore an overlay removed by a newer request. */
    if (started.tv_sec < state.seconds
        || (started.tv_sec == state.seconds
            && started.tv_nsec < state.nanoseconds)) {
        goto close_lock;
    }
    if (state.paused && action == STIV_FIFO_DRAW) {
        goto close_lock;
    }

    state.seconds = started.tv_sec;
    state.nanoseconds = started.tv_nsec;
    if (action == STIV_FIFO_PAUSE) {
        state.paused = true;
    } else if (action == STIV_FIFO_RESUME) {
        state.paused = false;
    }
    {
        ssize_t written = pwrite(lock_fd, &state, sizeof(state), 0);

        if (written != sizeof(state)) {
            err = written < 0 ? errno : EIO;
            goto close_lock;
        }
    }

    if (action != STIV_FIFO_RESUME) {
        err = -stiv_fifo_write(fifo, message, message_len);
    }

close_lock:
    if ((close(lock_fd) < 0) && (err == 0)) {
        err = errno;
    }
    return -err;
}

#endif /* STIV_FIFO_C */
