/* Based on GNU coreutils truncate by Pádraig Brady. */

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <ctype.h>

#include "gnulib/lib/intprops.h"

#ifndef OFF_T_MIN
#define OFF_T_MIN ((off_t) ~ OFF_T_MAX)
#endif

#ifndef OFF_T_MAX
#define OFF_T_MAX ((off_t) ((((off_t) 1 << ((sizeof (off_t) * CHAR_BIT) - 2)) - 1) * 2 + 1))
#endif

#ifndef S_TYPEISTMO
#define S_TYPEISTMO(p) 0
#endif

#include <locale.h>
#include <libintl.h>
#define _(String) gettext (String)

enum rel_mode
{
    rm_abs = 0,
    rm_rel,
    rm_min,
    rm_max,
    rm_rdn,
    rm_rup
};

static void error_tail (int status, int errnum, const char *message, va_list args)
{
    vfprintf (stderr, message, args);
    if (errnum)
    {
        fprintf (stderr, ": %s", strerror (errnum));
    }
    putc ('\n', stderr);
    fflush (stderr);
    if (status)
    {
        exit (status);
    }
}

void error (int status, int errnum, const char *message, ...)
{
    va_list args;
    fflush (stdout);
    fprintf (stderr, "%s: ", PROG_NAME);
    va_start (args, message);
    error_tail (status, errnum, message, args);
    va_end (args);
}

void usage (int status)
{
    if (status != EXIT_SUCCESS)
    {
        fprintf (stderr, _("Try '%s --help' for more information.\n"), PROG_NAME);
    }
    else
    {
        printf (_("Usage: %s OPTION... FILE...\n"), PROG_NAME);
        fputs (_("Shrink or extend the size of each FILE to the specified size\n\n"), stdout);
        fputs (_("A FILE argument that does not exist is created.\n\n"), stdout);
        fputs (_("If a FILE is larger than the specified size, the extra data is lost.\nIf a FILE is shorter, it is extended and the sparse extended part (hole)\nreads as zero bytes or padded with bytes with specified code.\n\n"), stdout);
        fputs (_("  -c, --no-create        do not create any files\n"), stdout);
        fputs (_("  -o, --io-blocks        treat SIZE as number of IO blocks instead of bytes\n"), stdout);
        fputs (_("  -r, --reference=RFILE  base size on RFILE\n"), stdout);
        fputs (_("  -s, --size=SIZE        set or adjust the file size by SIZE bytes\n"), stdout);
        fputs (_("  -C, --character=CODE   pad file with character of code CODE (no effect if new size is less than of equal to initial size) instead of leaving it as zero bytes\n"), stdout);
        fputs (_("  -h, --help             display this help and exit\n"), stdout);
        fputs (_("  -v, --version          output version information and exit\n"), stdout);
        fputs (_("\nThe SIZE argument is an integer and optional unit (example: 10K is 10*1024).\nUnits are K,M,G,T,P,E,Z,Y (powers of 1024) or KB,MB,... (powers of 1000).\nBinary prefixes can be used, too: KiB=K, MiB=M, and so on.\n"), stdout);
        fputs (_("\nSIZE may also be prefixed by one of the following modifying characters:\n'+' extend by, '-' reduce by, '<' at most, '>' at least,\n'/' round down to multiple of, '%' round up to multiple of.\n"), stdout);
        fputs (_("\nCODE may be a decimal number 0 to 255, hexadecimal 0x0 till 0xff, or octal 00 till 0400.\n"), stdout);
    }
    exit (status);
}

static inline int usable_st_size (struct stat const *sb)
{
    return (S_ISREG (sb->st_mode) || S_ISLNK (sb->st_mode) || S_TYPEISSHM (sb) || S_TYPEISTMO (sb));
}

/* return 1 on success, 0 on error.  */
static int do_ftruncate (int fd, char const *fname, off_t ssize, off_t rsize, enum rel_mode rel_mode, int char_code, int block_mode)
{
    struct stat sb;
    off_t nsize;
    int ret = 1;
    off_t fsize;

    if ((block_mode || (rel_mode && rsize < 0)) && fstat (fd, &sb) != 0)
    {
        error (0, errno, _("cannot fstat \"%s\""), fname);
        return 0;
    }

    if (block_mode)
    {
        ptrdiff_t blksize = sb.st_blksize;
        intmax_t ssize0 = ssize;
        if (INT_MULTIPLY_WRAPV (ssize, blksize, &ssize))
        {
            error (0, 0, _("overflow in %" PRIdMAX " * %" PRIdPTR " byte blocks for file \"%s\""), ssize0, blksize, fname);
            return 0;
        }
    }

    if (rel_mode || char_code >= 0)
    {

        if (0 <= rsize)
        {
            fsize = rsize;
        }
        else
        {
            if (usable_st_size (&sb))
            {
                fsize = sb.st_size;
                if (fsize < 0)
                {
                    /* Sanity check.  Overflow is the only reason I can think this would ever go negative. */
                    error (0, 0, _("\"%s\" has unusable, apparently negative size"), fname);
                    return 0;
                }
            }
            else
            {
                fsize = lseek (fd, 0, SEEK_END);
                if (fsize < 0)
                {
                    error (0, errno, _("cannot get the size of \"%s\""), fname);
                    return 0;
                }
            }
        }

        if (rel_mode == rm_min)
        {
            nsize = (fsize > ssize) ? fsize : ssize;
        }
        else if (rel_mode == rm_max)
        {
            nsize = (fsize < ssize) ? fsize : ssize;
        }
        else if (rel_mode == rm_rdn)
        {
            /* 0..ssize-1 -> 0 */
            nsize = fsize - fsize % ssize;
        }
        else if (rel_mode)
        {
            if (rel_mode == rm_rup)
            {
                /* 1..ssize -> ssize */
                off_t r = fsize % ssize;
                ssize = r == 0 ? 0 : ssize - r;
            }
            if (INT_ADD_WRAPV (fsize, ssize, &nsize))
            {
                error (0, 0, _("overflow extending size of file \"%s\""), fname);
                return 0;
            }
        }
    }
    else
    {
        nsize = ssize;
    }

    if (nsize < 0)
    {
        nsize = 0;
    }

    if (ftruncate (fd, nsize) != 0)
    {
        intmax_t s = nsize;
        error (0, errno, _("failed to truncate \"%s\" at %" PRIdMAX " bytes"), fname, s);
        return 0;
    }

    if (char_code >= 0)
    {
        char *blk;
        off_t pos;

        pos = lseek (fd, fsize, SEEK_SET);
        if (pos != fsize)
        {
            if (pos == (off_t)-1)
            {
                error (0, errno, _("failed to seek \"%s\""), fname);
                return 0;
            }
            else
            {
                error (0, 0, _("wrong seek position got for \"%s\""), fname);
                return 0;
            }
        }

        blk = malloc (sb.st_blksize);
        if (blk == NULL)
        {
            error (0, errno, _("failed to allocate block to fill"));
            return 0;
        }
        memset (blk, char_code, sb.st_blksize);

        for (nsize -= fsize; nsize > 0; nsize -= sb.st_blksize)
        {
            ssize_t rem = (nsize > sb.st_blksize) ? sb.st_blksize : nsize;
            ssize_t act = write (fd, blk, rem);
            if (act < 0)
            {
                error (0, errno, _("failure during writing \"%s\""), fname);
                ret = 0;
                break;
            }
            else if (act < rem)
            {
                error (0, 0, _("can't write block to \"%s\""), fname);
                ret = 0;
                break;
            }
        }
        free(blk);
    }

    return ret;
}

enum strtol_error
{
    LONGINT_OK = 0,
    LONGINT_OVERFLOW = 1,
    LONGINT_INVALID_SUFFIX_CHAR = 2,
    LONGINT_INVALID_SUFFIX_CHAR_WITH_OVERFLOW = (LONGINT_INVALID_SUFFIX_CHAR | LONGINT_OVERFLOW),
    LONGINT_INVALID = 4
};

enum strtol_error bkm_scale_by_power_custom (long int *x, int base, int power)
{
    enum strtol_error err = LONGINT_OK;
    while (power--)
    {
        long int scaled;
        if (INT_MULTIPLY_WRAPV (*x, base, &scaled))
        {
            *x = *x < 0 ? LONG_MIN : LONG_MAX;
            err = LONGINT_OVERFLOW;
        }
        *x = scaled;
    }
    return err;
}


enum strtol_error xstrtol_custom (const char *s, long int *val)
{
    char *t_ptr;
    long int tmp;
    enum strtol_error err = LONGINT_OK;
    errno = 0;
    tmp = strtol (s, &t_ptr, 10);

    if (t_ptr == s)
    {
        /* If there is no number but there is a valid suffix, assume the number is 1.  The string is invalid otherwise.  */
        if (*t_ptr && strchr ("EgGkKmMPtTYZ", *t_ptr))
        {
            tmp = 1;
        }
        else
        {
            return LONGINT_INVALID;
        }
    }
    else if (errno != 0)
    {
        if (errno != ERANGE)
        {
            return LONGINT_INVALID;
        }
        err = LONGINT_OVERFLOW;
    }

    if (*t_ptr != '\0')
    {
        int base = 1024;
        int suffixes = 1;
        enum strtol_error overflow;

        if (!strchr ("EgGkKmMPtTYZ", *t_ptr))
        {
            *val = tmp;
            return err | LONGINT_INVALID_SUFFIX_CHAR;
        }
        else
        {
            switch (t_ptr[1])
            {
                case 'i':
                    if (t_ptr[2] == 'B')
                    {
                        suffixes += 2;
                    }
                    break;

                case 'B':
                case 'D': /* 'D' is obsolescent */
                    base = 1000;
                    suffixes++;
                    break;
            }
        }

        switch (*t_ptr)
        {
            case 'E': /* exa or exbi */
                overflow = bkm_scale_by_power_custom (&tmp, base, 6);
                break;

            case 'G': /* giga or gibi */
            case 'g': /* 'g' is undocumented; for compatibility only */
                overflow = bkm_scale_by_power_custom (&tmp, base, 3);
                break;

            case 'k': /* kilo */
            case 'K': /* kibi */
                overflow = bkm_scale_by_power_custom (&tmp, base, 1);
                break;

            case 'M': /* mega or mebi */
            case 'm': /* 'm' is undocumented; for compatibility only */
                overflow = bkm_scale_by_power_custom (&tmp, base, 2);
                break;

            case 'P': /* peta or pebi */
                overflow = bkm_scale_by_power_custom (&tmp, base, 5);
                break;

            case 'T': /* tera or tebi */
            case 't': /* 't' is undocumented; for compatibility only */
                overflow = bkm_scale_by_power_custom (&tmp, base, 4);
                break;

            case 'Y': /* yotta or 2**80 */
                overflow = bkm_scale_by_power_custom (&tmp, base, 8);
                break;

            case 'Z': /* zetta or 2**70 */
                overflow = bkm_scale_by_power_custom (&tmp, base, 7);
                break;

            default:
                *val = tmp;
                return err | LONGINT_INVALID_SUFFIX_CHAR;
        }

        err |= overflow;
        t_ptr += suffixes;
        if (*t_ptr)
        {
            err |= LONGINT_INVALID_SUFFIX_CHAR;
        }
    }

    *val = tmp;
    return err;
}

off_t xdectoimax_custom (char const *n_str)
{
    enum strtol_error s_err;

    off_t tnum;
    s_err = xstrtol_custom (n_str, &tnum);

    if (s_err == LONGINT_OK)
    {
        if (tnum < OFF_T_MIN || OFF_T_MAX < tnum)
        {
            s_err = LONGINT_OVERFLOW;
            /* Use have the INT range as a heuristic to distinguish type overflow rather than other min/max limits.  */
            if (tnum > INT_MAX / 2)
            {
                errno = EOVERFLOW;
            }
            else if (tnum < INT_MIN / 2)
            {
                errno = EOVERFLOW;
            }
            else
            {
                errno = ERANGE;
            }
        }
    }
    else if (s_err == LONGINT_OVERFLOW)
    {
        errno = EOVERFLOW;
    }
    else if (s_err == LONGINT_INVALID_SUFFIX_CHAR_WITH_OVERFLOW)
    {
        errno = 0; /* Don't show ERANGE errors for invalid numbers.  */
    }

    if (s_err != LONGINT_OK)
    {
        /* EINVAL error message is redundant in this context.  */
        error (EXIT_FAILURE, errno == EINVAL ? 0 : errno, "%s: \"%s\"", _("Invalid number"), n_str);
    }

    return tnum;
}

int main (int argc, char **argv)
{
    int got_size = 0;
    off_t size;
    off_t rsize = -1;
    enum rel_mode rel_mode = rm_abs;
    int c;
    char *endp;
    int no_create = 0;
    int block_mode = 0;
    char const *ref_file = NULL;
    int char_code = -1;
    static struct option const longopts[] =
    {
        {"no-create", no_argument, 0, 'c'},
        {"io-blocks", no_argument, 0, 'o'},
        {"reference", required_argument, 0, 'r'},
        {"size", required_argument, 0, 's'},
        {"character", required_argument, 0, 'C'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {NULL, 0, 0, 0}
    };

    setlocale (LC_ALL, "");
    bindtextdomain (PROG_NAME, LOCALEDIR);
    textdomain (PROG_NAME);

    while ((c = getopt_long (argc, argv, "cor:s:C:hv", longopts, NULL)) != -1)
    {
        switch (c)
        {
            case 'c':
                no_create = 1;
                break;

            case 'o':
                block_mode = 1;
                break;

            case 'r':
                ref_file = optarg;
                break;

            case 'C':
                char_code = strtol(optarg,&endp,0);
                if(!endp || !*endp)
                {
                    error (0, errno, _("wrong character code argument %s"), optarg);
                    usage (EXIT_FAILURE);
                }
                if(char_code < 0 || char_code > 255)
                {
                    error (0, 0, _("wrong character code value %d (should be 0..255)"), char_code);
                    usage (EXIT_FAILURE);
                }
                break;

            case 's':
                /* skip any whitespace */
                while (isspace (*optarg))
                {
                    optarg++;
                }

                switch (*optarg)
                {
                    case '<':
                        rel_mode = rm_max;
                        optarg++;
                        break;

                    case '>':
                        rel_mode = rm_min;
                        optarg++;
                        break;

                    case '/':
                        rel_mode = rm_rdn;
                        optarg++;
                        break;

                    case '%':
                        rel_mode = rm_rup;
                        optarg++;
                        break;
                }

                /* skip any whitespace */
                while (isspace (*optarg))
                {
                    optarg++;
                }

                if (*optarg == '+' || *optarg == '-')
                {
                    if (rel_mode)
                    {
                        error (0, 0, _("multiple relative modifiers specified"));
                        /* Note other combinations are flagged as invalid numbers */
                        usage (EXIT_FAILURE);
                    }
                    rel_mode = rm_rel;
                }

                /* Support dd BLOCK size suffixes + lowercase g,t,m for bsd compat. Note we don't support dd's b=512, c=1, w=2 or 21x512MiB formats. */
                size = xdectoimax_custom (optarg);

                /* Rounding to multiple of 0 is nonsensical */
                if ((rel_mode == rm_rup || rel_mode == rm_rdn) && size == 0)
                {
                    error (EXIT_FAILURE, 0, _("division by zero"));
                }

                got_size = 1;
                break;

            case 'h':
                usage (EXIT_SUCCESS);
                break;

            case 'v':
                printf (_("%s version %s\n"), PROG_NAME, PROG_VERSION);
                exit (EXIT_SUCCESS);
                break;

            default:
                usage (EXIT_FAILURE);
        }
    }

    argv += optind;
    argc -= optind;

    /* must specify either size or reference file */
    if (!ref_file && !got_size)
    {
        error (0, 0, _("you must specify either --size or --reference"));
        usage (EXIT_FAILURE);
    }

    /* must specify a relative size with a reference file */
    if (ref_file && got_size && !rel_mode)
    {
        error (0, 0, _("you must specify a relative --size with --reference"));
        usage (EXIT_FAILURE);
    }

    /* block_mode without size is not valid */
    if (block_mode && !got_size)
    {
        error (0, 0, _("--io-blocks was specified but --size was not"));
        usage (EXIT_FAILURE);
    }

    /* must specify at least 1 file */
    if (argc < 1)
    {
        error (0, 0, _("missing file operand"));
        usage (EXIT_FAILURE);
    }

    if (ref_file)
    {
        struct stat sb;
        off_t file_size = -1;
        if (stat (ref_file, &sb) != 0)
        {
            error (EXIT_FAILURE, errno, _("cannot stat \"%s\""), ref_file);
        }
        if (usable_st_size (&sb))
        {
            file_size = sb.st_size;
        }
        else
        {
            int ref_fd = open (ref_file, O_RDONLY);
            if (0 <= ref_fd)
            {
                off_t file_end = lseek (ref_fd, 0, SEEK_END);
                int saved_errno = errno;
                close (ref_fd); /* ignore failure */
                if (0 <= file_end)
                {
                    file_size = file_end;
                }
                else
                {
                  /* restore, in case close clobbered it. */
                  errno = saved_errno;
                }
            }
        }
        if (file_size < 0)
        {
            error (EXIT_FAILURE, errno, _("cannot get the size of \"%s\""), ref_file);
        }
        if (!got_size)
        {
            size = file_size;
        }
        else
        {
            rsize = file_size;
        }
    }

    int oflags = O_WRONLY | (no_create ? 0 : O_CREAT) | O_NONBLOCK;
    int errors = 0;

    for (char const *fname; (fname = *argv); argv++)
    {
        int fd = open (fname, oflags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd < 0)
        {
            /* 'truncate -s0 -c no-such-file'  shouldn't gen error
               'truncate -s0 no-such-dir/file' should gen ENOENT error
               'truncate -s0 no-such-dir/' should gen EISDIR error
               'truncate -s0 .' should gen EISDIR error */
            if (!(no_create && errno == ENOENT))
            {
                error (0, errno, _("cannot open \"%s\" for writing"), fname);
                errors = 1;
            }
        }
        else
        {
            errors |= !do_ftruncate (fd, fname, size, rsize, rel_mode, char_code, block_mode);
            if (close (fd) != 0)
            {
                error (0, errno, _("failed to close \"%s\""), fname);
                errors = 1;
            }
        }
    }

    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}
