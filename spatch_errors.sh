#!/bin/bash
set -eu

# Usage:
#   for dir in mm net fs kernel security sound virt lib ipc include; do ./spatch_errors.sh --dir $dir --include-headers --in-place -j 4; done

# We skip arch/ as there's some boot code that doesn't link our set_last_err() function.
# We also skip driver/ because it's unlikely to be useful.

# We also skip -ENOMEM and -EFAULT, because these introduce noise.

IGNORE_ERRORS='(ENOMEM|EFAULT)'

ERRORS=$(
cat include/uapi/asm-generic/errno.h \
    include/uapi/asm-generic/errno-base.h \
    include/linux/errno.h \
  | grep '^#define\s\+E[A-Z0-9]\+\s\+[0-9]\+' \
  | awk '{print $2}' \
  | sort -u \
  | grep -Ev $IGNORE_ERRORS \
  | xargs ruby -e 'puts ARGV.map { |e| "#{e}@p" }.join("\\|")'
)

cat > /tmp/patch.cocci <<- EOF
@e@
expression var1;
type T;
identifier var2;
position p;
@@

/*
 * We patch errors used as rvalues.
 * avoiding errors used in if statements, and switch/case statments.
 */

(
var1 = <+... \($ERRORS\) ...+>;
|
T var2 = <+... \($ERRORS\) ...+>;
|
return <+... \($ERRORS\) ...+>;
)

@@
expression E;
position e.p;
@@

- E@p
+ ERR(E)
EOF

exec spatch --sp-file /tmp/patch.cocci "$@"
