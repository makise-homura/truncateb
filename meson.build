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

test('truncate to same size', test_wrapper, args: ['test0', 'a5274d87054ad92cb5ceea1305b63420', truncateb.full_path(), srcfile, '-s 8192'])
test('truncate to lesser size', test_wrapper, args: ['test1', '9d9781e1b9251d310026056b616ddc6b', truncateb.full_path(), srcfile, '-s 4096'])
test('truncate to greater size', test_wrapper, args: ['test2', '759000319ee7a3330459cdfe461da40f', truncateb.full_path(), srcfile, '-s 16384'])
test('truncate to greater size padding zero bytes', test_wrapper, args: ['test3', '759000319ee7a3330459cdfe461da40f', truncateb.full_path(), srcfile, '-s 16384 -C 0'])
test('truncate to greater size padding 0xFF bytes', test_wrapper, args: ['test4', 'd5810a9eabe5bbe99bd50e6bc45c7770', truncateb.full_path(), srcfile, '-s 16384 -C 0xFF'])
test('truncate to same size using reference file', test_wrapper, args: ['test5', 'a5274d87054ad92cb5ceea1305b63420', truncateb.full_path(), srcfile, '-r ' + reffile + '8k'])
test('truncate to lesser size using reference file', test_wrapper, args: ['test6', '9d9781e1b9251d310026056b616ddc6b', truncateb.full_path(), srcfile, '-r ' + reffile + '4k'])
test('truncate to greater size using reference file', test_wrapper, args: ['test7', '759000319ee7a3330459cdfe461da40f', truncateb.full_path(), srcfile, '-r ' + reffile + '16k'])
test('truncate to greater size padding zero bytes using reference file', test_wrapper, args: ['test8', '759000319ee7a3330459cdfe461da40f', truncateb.full_path(), srcfile, '-r ' + reffile + '16k -C 0'])
test('truncate to greater size padding 0xFF bytes using reference file', test_wrapper, args: ['test9', 'd5810a9eabe5bbe99bd50e6bc45c7770', truncateb.full_path(), srcfile, '-r ' + reffile + '16k -C 0xFF'])
