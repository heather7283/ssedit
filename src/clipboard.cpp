#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <climits>
#include <unistd.h>
#include <fcntl.h>

#include "clibpoard.hpp"
#include "utils.hpp"
#include "log.hpp"

// This uses wl-copy, not because I'm lazy, but because due to the way wayland
// works clipboard contents will disappear once ssedit is closed.
// wl-copy forks itself in the background and clipboard will persist.
bool CopyToClipboard(Format format, const unsigned char *buf, size_t buf_size) {
    const char *tmpdir;
    int tmpfile_fd = -1;

    tmpdir = getenv("TMPDIR");
    if (tmpdir == nullptr) {
        tmpdir = "/tmp";
    }

    tmpfile_fd = open(tmpdir, O_RDWR | O_TMPFILE, S_IRUSR | S_IWUSR);
    if (tmpfile_fd < 0) {
        LogPrint(ERR, "Clipboard: failed to open temporary file (%s)", strerror(errno));
        goto err;
    }
    if (ftruncate(tmpfile_fd, buf_size) < 0) {
        LogPrint(ERR, "Clipboard: failed to truncate temporary file (%s)", strerror(errno));
        goto err;
    }

    if (!WriteToFD(tmpfile_fd, buf, buf_size)) {
        LogPrint(ERR, "Clipboard: writing clipboard contents to temporary file failed");
        goto err;
    }
    if (lseek(tmpfile_fd, 0, SEEK_SET) < 0) {
        LogPrint(ERR, "Clipboard: failed to rewird tmpfile position (%s)", strerror(errno));
        goto err;
    };

    pid_t pid;
    switch (pid = fork()) {
    case -1: // Error
        LogPrint(ERR, "Clipboard: failed to fork child (%s)", strerror(errno));
        goto err;
    case 0: // Child
        // Redirect stdin from temporary file
        if (dup2(tmpfile_fd, 0) < 0) {
            LogPrint(ERR, "Clipboard: failed to redirect stdin (%s)", strerror(errno));
            exit(69);
        }
        execlp("wl-copy", "wl-copy", "-t", FormatToMIME(format), nullptr);
        LogPrint(ERR, "Clipboard: failed to exec into wl-copy (%s)", strerror(errno));
        exit(69);
    default: // Parent
        break;
    }

    close(tmpfile_fd);

err:
    if (tmpfile_fd > 0) {
        close(tmpfile_fd);
    }
    return false;
}

