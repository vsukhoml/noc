// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

/// @file errno.h
/// @brief Error handling

#ifndef NOC_ERRNO_H
#define NOC_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EPERM = 0x01,         // Operation not permitted
    ENOENT = 0x02,        // No such file or directory
    ESRCH = 0x03,         // No such process
    EINTR = 0x04,         // Interrupted system call
    EIO = 0x05,           // Input/output error
    ENXIO = 0x06,         // No such device or address
    E2BIG = 0x07,         // Argument list too long
    ENOEXEC = 0x08,       // Exec format error
    EBADF = 0x09,         // Bad file descriptor
    ECHILD = 0x0a,        // No child processes
    EAGAIN = 0x0b,        // Resource temporarily unavailable
    ENOMEM = 0x0c,        // Cannot allocate memory
    EACCES = 0x0d,        // Permission denied
    EFAULT = 0x0e,        // Bad address
    ENOTBLK = 0x0f,       // Block device required
    EBUSY = 0x10,         // Device or resource busy
    EEXIST = 0x11,        // File exists
    EXDEV = 0x12,         // Invalid cross-device link
    ENODEV = 0x13,        // No such device
    ENOTDIR = 0x14,       // Not a directory
    EISDIR = 0x15,        // Is a directory
    EINVAL = 0x16,        // Invalid argument
    ENFILE = 0x17,        // Too many open files in system
    EMFILE = 0x18,        // Too many open files
    ENOTTY = 0x19,        // Inappropriate ioctl for device
    ETXTBSY = 0x1a,       // Text file busy
    EFBIG = 0x1b,         // File too large
    ENOSPC = 0x1c,        // No space left on device
    ESPIPE = 0x1d,        // Illegal seek
    EROFS = 0x1e,         // Read-only file system
    EMLINK = 0x1f,        // Too many links
    EPIPE = 0x20,         // Broken pipe
    EDOM = 0x21,          // Numerical argument out of domain
    ERANGE = 0x22,        // Numerical result out of range
    EDEADLOCK = 0x23,     // Resource deadlock avoided
    ENAMETOOLONG = 0x24,  // File name too long
    ENOLCK = 0x25,        // No locks available
    ENOSYS = 0x26,        // Function not implemented
    ENOTEMPTY = 0x27,     // Directory not empty
    ELOOP = 0x28,         // Too many levels of symbolic links
    ENOMSG = 0x2a,        // No message of desired type
    EIDRM = 0x2b,         // Identifier removed
    ECHRNG = 0x2c,        // Channel number out of range
    EL2NSYNC = 0x2d,      // Level 2 not synchronized
    EL3HLT = 0x2e,        // Level 3 halted
    EL3RST = 0x2f,        // Level 3 reset
    ELNRNG = 0x30,        // Link number out of range
    EUNATCH = 0x31,       // Protocol driver not attached
    ENOCSI = 0x32,        // No CSI structure available
    EL2HLT = 0x33,        // Level 2 halted
    EBADE = 0x34,         // Invalid exchange
    EBADR = 0x35,         // Invalid request descriptor
    EXFULL = 0x36,        // Exchange full
    ENOANO = 0x37,        // No anode
    EBADRQC = 0x38,       // Invalid request code
    EBADSLT = 0x39,       // Invalid slot
    EBFONT = 0x3b,        // Bad font file format
    ENOSTR = 0x3c,        // Device not a stream
    ENODATA = 0x3d,       // No data available
    ETIME = 0x3e,         // Timer expired
    ENOSR = 0x3f,         // Out of streams resources
    ENONET = 0x40,        // Machine is not on the network
    ENOPKG = 0x41,        // Package not installed
    EREMOTE = 0x42,       // Object is remote
    ENOLINK = 0x43,       // Link has been severed
    EADV = 0x44,          // Advertise error
    ESRMNT = 0x45,        // Srmount error
    ECOMM = 0x46,         // Communication error on send
    EPROTO = 0x47,        // Protocol error
    EMULTIHOP = 0x48,     // Multihop attempted
    EDOTDOT = 0x49,       // RFS specific error
    EBADMSG = 0x4a,       // Bad message
    EOVERFLOW = 0x4b,     // Value too large for defined data type
    ENOTUNIQ = 0x4c,      // Name not unique on network
    EBADFD = 0x4d,        // File descriptor in bad state
    EREMCHG = 0x4e,       // Remote address changed
    ELIBACC = 0x4f,       // Can not access a needed shared library
    ELIBBAD = 0x50,       // Accessing a corrupted shared library
    ELIBSCN = 0x51,       //.lib section in a.out corrupted
    ELIBMAX = 0x52,       // Attempting to link in too many shared libraries
    ELIBEXEC = 0x53,      // Cannot exec a shared library directly
    EILSEQ = 0x54,        // Invalid or incomplete multibyte or wide character
    ERESTART = 0x55,      // Interrupted system call should be restarted
    ESTRPIPE = 0x56,      // Streams pipe error
    EUSERS = 0x57,        // Too many users
    ENOTSOCK = 0x58,      // Socket operation on non-socket
    EDESTADDRREQ = 0x59,  // Destination address required
    EMSGSIZE = 0x5a,      // Message too long
    EPROTOTYPE = 0x5b,    // Protocol wrong type for socket
    ENOPROTOOPT = 0x5c,   // Protocol not available
    EPROTONOSUPPORT = 0x5d,  // Protocol not supported
    ESOCKTNOSUPPORT = 0x5e,  // Socket type not supported
    EOPNOTSUPP = 0x5f,       // Operation not supported
    EPFNOSUPPORT = 0x60,     // Protocol family not supported
    EAFNOSUPPORT = 0x61,     // Address family not supported by protocol
    EADDRINUSE = 0x62,       // Address already in use
    EADDRNOTAVAIL = 0x63,    // Cannot assign requested address
    ENETDOWN = 0x64,         // Network is down
    ENETUNREACH = 0x65,      // Network is unreachable
    ENETRESET = 0x66,        // Network dropped connection on reset
    ECONNABORTED = 0x67,     // Software caused connection abort
    ECONNRESET = 0x68,       // Connection reset by peer
    ENOBUFS = 0x69,          // No buffer space available
    EISCONN = 0x6a,          // Transport endpoint is already connected
    ENOTCONN = 0x6b,         // Transport endpoint is not connected
    ESHUTDOWN = 0x6c,        // Cannot send after transport endpoint shutdown
    ETOOMANYREFS = 0x6d,     // Too many references: cannot splice
    ETIMEDOUT = 0x6e,        // Connection timed out
    ECONNREFUSED = 0x6f,     // Connection refused
    EHOSTDOWN = 0x70,        // Host is down
    EHOSTUNREACH = 0x71,     // No route to host
    EALREADY = 0x72,         // Operation already in progress
    EINPROGRESS = 0x73,      // Operation now in progress
    ESTALE = 0x74,           // Stale file handle
    EUCLEAN = 0x75,          // Structure needs cleaning
    ENOTNAM = 0x76,          // Not a XENIX named type file
    ENAVAIL = 0x77,          // No XENIX semaphores available
    EISNAM = 0x78,           // Is a named type file
    EREMOTEIO = 0x79,        // Remote I/O error
    EDQUOT = 0x7a,           // Disk quota exceeded
    ENOMEDIUM = 0x7b,        // No medium found
    EMEDIUMTYPE = 0x7c,      // Wrong medium type
    ECANCELED = 0x7d,        // Operation canceled
    ENOKEY = 0x7e,           // Required key not available
    EKEYEXPIRED = 0x7f,      // Key has expired
    EKEYREVOKED = 0x80,      // Key has been revoked
    EKEYREJECTED = 0x81,     // Key was rejected by service
    EOWNERDEAD = 0x82,       // Owner died
    ENOTRECOVERABLE = 0x83,  // State not recoverable
    ERFKILL = 0x84,          // Operation not possible due to RF-kill
    EHWPOISON = 0x85,        // Memory page has hardware error
    EOTHER = 0x90,           // Other
} errno_t;

#define EWOULDBLOCK EAGAIN
#define EDEADLK EDEADLOCK

// Thread-specific variable with number of last error.
extern __thread volatile errno_t errno;

#ifdef __cplusplus
}
#endif

#endif /* NOC_ERRNO_H */