project('truncateb', 'c', version: 'v0.1-dev')

# E-mail to insert to .pot file
email = 'akemi_homura@kurisa.ch'

# Determine version from git
ver = meson.project_version()
git = find_program('git', required: false)
if git.found()
    git_describe_cmd = run_command(git, 'describe', '--dirty')
    if git_describe_cmd.returncode() == 0
        ver = git_describe_cmd.stdout().strip()
    endif
endif
message('determined version ' + ver)

# Internationalization support
subdir('po')
localedir = get_option('prefix') / get_option('localedir')

# Generate documentation
ronn = find_program('ronn', required: false)
if ronn.found()
    custom_target('doc-man', input : files('truncateb.1.ronn'), output : 'truncateb.1', build_by_default : true, command : [ronn, '--output-dir', '@OUTDIR@', '--roff', '@INPUT@'], install : true, install_dir: get_option('mandir') / 'man1')
    custom_target('readme', input : files('truncateb.1.ronn'), output : 'README.md', capture: true, command : [ronn, '--pipe', '--markdown', '@INPUT@'])
else
    warning('ronn program not found, documentation will not be generated')
endif

truncateb = executable('truncateb', 'truncateb.c', install : true, c_args: ['-DPROG_NAME="' + meson.project_name() + '"', '-DPROG_VERSION="' + ver + '"', '-DLOCALEDIR="' + localedir + '"'])

test_wrapper = find_program('tests/test_wrapper.py')

srcfile = meson.current_source_dir() / 'tests/srcfile'
reffile = meson.current_source_dir() / 'tests/reffile.'

testdata = {
    'truncate to zero size':     {'args': '-s 0',     'rfargs': '-r ' + reffile + '0k',  'md5': 'd41d8cd98f00b204e9800998ecf8427e'},
    'truncate to lesser size':   {'args': '-s 4096',  'rfargs': '-r ' + reffile + '4k',  'md5': '9d9781e1b9251d310026056b616ddc6b'},
    'truncate to same size':     {'args': '-s 8192',  'rfargs': '-r ' + reffile + '8k',  'md5': 'a5274d87054ad92cb5ceea1305b63420'},
    'truncate to greater size':  {'args': '-s 16384', 'rfargs': '-r ' + reffile + '16k', 'md5': '759000319ee7a3330459cdfe461da40f', 'md5ff': 'd5810a9eabe5bbe99bd50e6bc45c7770'}
}

foreach testname, data : testdata
    if 'md5ff' in data
        md5ff = data['md5ff']
    else
        md5ff = data['md5']
    endif

    if 'args' in data
        test(testname,                         test_wrapper, args: [testname.underscorify() + '-a', data['md5'], truncateb.full_path(), srcfile, data['args']])
        test(testname + ' padding zero bytes', test_wrapper, args: [testname.underscorify() + '-z', data['md5'], truncateb.full_path(), srcfile, data['args'] + ' -C 0'])
        test(testname + ' padding 0xFF bytes', test_wrapper, args: [testname.underscorify() + '-f', md5ff,       truncateb.full_path(), srcfile, data['args'] + ' -C 0xFF'])
    endif
    if 'rfargs' in data
        test(testname + ' using reference file',                    test_wrapper, args: [testname.underscorify() + '-ar', data['md5'], truncateb.full_path(), srcfile, data['rfargs']])
        test(testname + ' padding zero bytes using reference file', test_wrapper, args: [testname.underscorify() + '-zr', data['md5'], truncateb.full_path(), srcfile, data['rfargs'] + ' -C 0'])
        test(testname + ' padding 0xFF bytes using reference file', test_wrapper, args: [testname.underscorify() + '-fr', md5ff,       truncateb.full_path(), srcfile, data['rfargs'] + ' -C 0xFF'])
    endif
endforeach

# test system TODO:
#   generate reference files
#   check -o
#   check -c on existing file
#   check -c on not-existing file
#   check -c on existing file
#   check '+' extend by, '-' reduce by, '<' at most, '>' at least, '/' round down to multiple of, '%' round up to multiple of.
