truncateb(1) -- Shrink or extend the size of a file to the specified size, possibly padding by a specified byte
===============================================================================================================

## SYNOPSIS

`truncateb` <var>OPTION</var>... <var>FILE</var>...

## DESCRIPTION

Shrink or extend the size of each FILE to the specified size

A <var>FILE</var> argument that does not exist is created.

If a <var>FILE</var> is larger than the specified size, the extra data is lost.
If a <var>FILE</var> is shorter, it is extended and the sparse extended part (hole)
reads as zero bytes or padded with bytes with specified code.

## OPTIONS

* `-c`, `--no-create`:
    do not create any files

* `-o`, `--io-blocks`:
    treat SIZE as number of IO blocks instead of bytes

* `-r`, `--reference` <var>RFILE</var>:
    base size on RFILE

* `-s`, `--size` <var>SIZE</var>:
    set or adjust the file size by SIZE bytes

* `-C`, `--character` <var>CODE</var>:
    pad file with character of code CODE (no effect if new size is less than of equal to initial size) instead of leaving it as zero bytes

* `-h`, `--help`:
    display help and exit

* `-v`, `--version`:
    output version information and exit

The <var>SIZE</var> argument is an integer and optional unit (example: 10K is 10*1024).
Units are K,M,G,T,P,E,Z,Y (powers of 1024) or KB,MB,... (powers of 1000).
Binary prefixes can be used, too: KiB=K, MiB=M, and so on.

<var>SIZE</var> may also be prefixed by one of the following modifying characters:
'+' extend by, '-' reduce by, '<var>' at most, '</var>' at least,
'/' round down to multiple of, '%' round up to multiple of.

<var>CODE</var> may be a decimal number 0 to 255, hexadecimal 0x0 till 0xff, or octal 00 till 0400.

Mandatory arguments to long options are mandatory for short options too.

## BUILDING FROM SOURCE

Source code is available in GitHub repository:
https://github.com/makise-homura/truncateb

You will need meson(1), gettext(1), ninja(1), and preferably
git(1) to determine version, and ronn(1) to generate documentation.

```
git clone https://github.com/makise-homura/truncateb
cd truncateb
git submodule update --init
mkdir build
cd build
meson ..
ninja
meson test
sudo ninja install
```

You may add `-Dprefix=`<var>INSTALL PREFIX</var> to `meson` invocation;
usually it is `/usr`. If not specified, defaults to `/usr/local`.

To update `README.md`, also run `ninja README.md`,
then copy resulting file to source tree.

If you're interested in writing your own translations, refer to
manual of `i18n` module of meson(1). Targets of subject are:

* To update pot files, run `ninja syncp-pot`.

* To update po files, run `ninja syncp-update-po`.

## AUTHOR

This software is based on truncate(1) from GNU coreutils,
which is written by Padraig Brady.

Padding with specified byte part is written by Igor Molchanov (aka makise-homura).

## REPORTING BUGS

Feel free to open issues and pull requests in GitHub repository:
https://github.com/makise-homura/truncateb

## SEE ALSO
       truncate(1), dd(1), truncate(2), ftruncate(2)


[SYNOPSIS]: #SYNOPSIS "SYNOPSIS"
[DESCRIPTION]: #DESCRIPTION "DESCRIPTION"
[OPTIONS]: #OPTIONS "OPTIONS"
[BUILDING FROM SOURCE]: #BUILDING-FROM-SOURCE "BUILDING FROM SOURCE"
[AUTHOR]: #AUTHOR "AUTHOR"
[REPORTING BUGS]: #REPORTING-BUGS "REPORTING BUGS"
[SEE ALSO]: #SEE-ALSO "SEE ALSO"


[truncateb(1)]: truncateb.1.html
