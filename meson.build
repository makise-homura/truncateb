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

executable('truncateb', 'truncateb.c', install : true, c_args: ['-DPROG_NAME="' + meson.project_name() + '"', '-DPROG_VERSION="' + ver + '"', '-DLOCALEDIR="' + localedir + '"'])
